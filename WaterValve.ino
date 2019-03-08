#include <SPI.h>              // needed for Arduino versions later than 0018
#include <Ethernet.h>         //library for ethernet shield
#include <EthernetUdp.h>      // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <TimeLib.h>

// temperature and motor
#include <OneWire.h>        
#include <DallasTemperature.h>  //Dallas DS18B20 temperature sensor library
#include <AFMotor.h>            //for controlling valve

AF_DCMotor valve(1);            //instantiate valve
OneWire sensor(30);             //instantiate onewire object
DallasTemperature sensors(&sensor);         //instantiate temperature sensor using onewire object

int water = 0;                //flag for whether valve is open or closed
int alert = 0;                //whether to warn the humans about water being too hot

float lowerTempLimit = 21.5;    //switch valve off for temperature below (too cold and condensation will form on instruments)
float upperTempLimit = 23;      //switch valve on for temperature above (we need to cool)
float warnTempLimit  = 25;      //warning TTL for temperature above

unsigned long delayTime      = 30000;    //delay between loops in ms
float temp;                   //temperature from sensor

/****************************************
    network stuff
*********************************************/

IPAddress timeServer(130, 159, 248, 123);  //Strathclyde time server
const int timeZone = 0;                    //Central European Time

byte mac[] = { 0xAA, 0xDE, 0xBE, 0xCF, 0xFD, 0xDD };  //local MAC address, fake
IPAddress localIPNum(172, 16, 1, 24);                 //local IP
byte gatewayIPNum[] = { 172, 16, 0, 1 }; 
byte dnsIPNum[]  = { 8, 8, 8, 8 }; 
byte maskIPNum[] = { 255, 255, 0, 0 }; 

IPAddress outIP(172,16,1,10);                   //IP address for data collector
unsigned int outPort = 8010;                    //port

EthernetUDP Udp;                                //instantiate UDP object for sending UDP packets

String nowStr;                                  //string for storing UNIX time
String str0;                                    //string for data to send

char UDPBuffer[512];                            //buffer to send string with UDP  

long int counter;                               //
int counterStartCold = 4;                       //



void setup(void)
{
  pinMode(31, OUTPUT);                          //
  digitalWrite(31, HIGH);                       //
  Serial.begin(9600);                           //

  valve.setSpeed(255);                          //Use maximum current for valve
  sensors.begin();                              //initialise temperature sensor
  sensors.setResolution(16);                    //16 bit precision

  counter = 0;                                  //

  Serial.println("Get NTP timer.");  
  
  Ethernet.begin(mac, localIPNum, dnsIPNum, gatewayIPNum, maskIPNum ); //Initialise Ethernet with network parameters we defined before
  Udp.begin(8888);                                                     //Begin UDP on port 8888                 

  //debug
  Serial.print("Local IP number: ");
  Serial.println(Ethernet.localIP() );
  Serial.println("Waiting time sync.");
  setSyncProvider(getNtpTime);  
  setSyncInterval(86400);  

  while (timeStatus() == timeNotSet) {      //until we get a time
    Serial.println("Waiting time sync.");   //print this every 3 seconds for debug
    delay(3000);
  }    
}

  
void loop(void)
{

  sensors.requestTemperatures();            //send request for temperature
  temp = sensors.getTempCByIndex(0);        //collect response and covert to degC
  Serial.print("Current temperature: ");    //debug
  Serial.print( temp );
  Serial.print(" degC,  Counter:");
  Serial.println( counter );
  
  if ((water == 1) && (temp < lowerTempLimit)) {        //If water is running, and temperature is too cold
    valve.run(RELEASE);                                 //close valve
    water = 0;                                          //remember that water is not flowing
    Serial.println("Close valve.");                     //debug
  }
  
  if ((water == 0) && (temp > upperTempLimit)) {        //if water is not running and temperature gets too hot
      valve.run(FORWARD);                               //open valve
      water = 1;                                        //remember that water is flowing
      Serial.println("Open valve.");                    //debug
  } 

  if (temp > warnTempLimit)  {                          //if temperature gets too hot
      alert = 1;                                        //alert mode
      digitalWrite(31, LOW);                            //
      Serial.println("High temperature. TTL alter on.");    //debug
  }
  if ((alert == 1) && (temp < warnTempLimit)) {         //if alert mode and the temperature isn't a worry anymore
      alert = 0;                                        //turn off alert mode
      digitalWrite(31, HIGH);                           
      Serial.println("High temperature gone. TTL alter off.");  
  }


  /*******************************************
     send network package with temperature
  *******************************************/
  
  nowStr = String( now() );                         //get time from NTP
  str0 = String("@T-CoilWater") + "#" + nowStr +  "#" + String( temp ) + 
            String("@T-CoilWater_Valve") + "#" + nowStr +  "#" + String( water ) + 
            String("@T-CoilWater_Warn") + "#" + nowStr +  "#" + String( alert );    //build string with all necessary information
   
   str0.toCharArray( UDPBuffer, str0.length()+1 );          //convert to char array
   Serial.println( UDPBuffer );                             //debug
   
   Udp.beginPacket( outIP, outPort );                       //get ready to transfer data over defined IP and port
   Udp.write(UDPBuffer);                                    //send the data
   Udp.endPacket();                                         //finish transfer

   // check pulse
   if ((counter > counterStartCold) && (water==0) && (temp>21.85)) {
      
      Serial.println("Counter pulse on.");
        valve.run(FORWARD);
        water = 1;
      delay(delayTime);
      delay(delayTime);
        valve.run(RELEASE);
        water = 0;   
      Serial.println("Counter pulse off.");  
      counter = 0;
  } else {
    delay(delayTime);
    counter++;
  }
  
}


/**<--------------------  NTP code --------------------->*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; //discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();   
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  //read packet into the buffer
      unsigned long secsSince1900;      
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //need to convert from secs since 1900 to secs since 1970 so Linux OS can convert more easily
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


