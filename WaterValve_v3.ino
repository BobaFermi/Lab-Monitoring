/*****************************************************************************
* Read temperature with Dallas sensors and open or close valve for cooling water
%
* +send to network server with udp
* 
* use mega and network shield, motor shield
*
******************************************************************************/

// network
#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <TimeLib.h>

// temparture and motor
#include <OneWire.h>
#include <DallasTemperature.h>
#include <AFMotor.h>

AF_DCMotor valve(1);
OneWire sensor(30);
DallasTemperature sensors(&sensor);

int water = 0;                // flag valve open/closed
int alert = 0;

float lowerTempLimit = 21.5;    // switch valve off for T below
float upperTempLimit = 23;      // switch valve on for T above
float warnTempLimit  = 25;      // warning TTL for T above

unsigned long delayTime      = 30000;    // delay between loops in ms
float temp;                   // temperature from sensor


/****************************************
    network stuff
*********************************************/

// NTP Servers:
IPAddress timeServer(130, 159, 248, 123);  // strathclyde
const int timeZone = 0;  // Central European Time

// local MAC address, fake
byte mac[] = { 0xAA, 0xDE, 0xBE, 0xCF, 0xFD, 0xDD };
// local ip
IPAddress localIPNum(172, 16, 1, 24);
byte gatewayIPNum[] = { 172, 16, 0, 1 }; 
byte dnsIPNum[]  = { 8, 8, 8, 8 }; 
byte maskIPNum[] = { 255, 255, 0, 0 }; 


// data collection server
IPAddress outIP(172,16,1,10);
unsigned int outPort = 8010;

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

/* send with netcat in linux: echo -n "@YUN#3335.4e19#234.11" | netcat -c -w1 -u 192.168.100.1 8010 */
//String s1   = "echo -n \"@";
//String s2   = "\" | netcat -c -w1 -u 172.16.0.1 8010 ";

String nowStr;
String str0;    

// buffer to send string with UDP
char UDPBuffer[512];          
  

long int counter;
int counterStartCold = 4;

/***************************
/*   setup 
/****************************/

void setup(void)
{
  pinMode(31, OUTPUT);
  digitalWrite(31, HIGH);
  Serial.begin(9600);

  valve.setSpeed(255);
  sensors.begin();
  sensors.setResolution(16);

 counter = 0;

  // network  
  Serial.println("Get NTP timer.");  
  // start the Ethernet and UDP:
  Ethernet.begin(mac, localIPNum, dnsIPNum, gatewayIPNum, maskIPNum );
  Udp.begin(8888);

  Serial.print("Local IP number: ");  
  Serial.println(Ethernet.localIP() );
  Serial.println("Waiting time sync.");
  setSyncProvider(getNtpTime);  
  setSyncInterval(86400);  

  while (timeStatus() == timeNotSet) {
    Serial.println("Waiting time sync.");    
    delay(3000);
  }    
  
}

/*********************************************
/*  loop
/* read temperature and 
/* open or close valve with motor shield
/**********************************************/
  
void loop(void)
{
  
  // get temperatures
  sensors.requestTemperatures(); 
  temp = sensors.getTempCByIndex(0);
  Serial.print("Current temperature: ");
  Serial.print( temp );
  Serial.print(" degC,  Counter:");
  Serial.println( counter );
  
  // check switch valve off?
  if ((water == 1) && (temp < lowerTempLimit))   {
    valve.run(RELEASE);
    water = 0;    
    Serial.println("Close valve.");    
  }
  
  if ((water == 0) && (temp > upperTempLimit)) {
      valve.run(FORWARD);
      water = 1;
      Serial.println("Open valve.");          
  } 

  if (temp > warnTempLimit)  {
      alert = 1;
      digitalWrite(31, LOW);
      Serial.println("High temperature. TTL alter on.");  
  }
  if ((alert == 1) && (temp < warnTempLimit)) {
      alert = 0;
      digitalWrite(31, HIGH);
      Serial.println("High temperature gone. TTL alter off.");  
  }


  /*******************************************
  // send network package with temperature
  *******************************************/
  
  // get time now
  nowStr = String( now() );
  str0 = String("@T-CoilWater") + "#" + nowStr +  "#" + String( temp ) + 
            String("@T-CoilWater_Valve") + "#" + nowStr +  "#" + String( water ) + 
            String("@T-CoilWater_Warn") + "#" + nowStr +  "#" + String( alert );
   
   str0.toCharArray( UDPBuffer, str0.length()+1 );
   Serial.println( UDPBuffer );
   
   Udp.beginPacket( outIP, outPort );
   Udp.write(UDPBuffer);
   Udp.endPacket();

   // check counter pulse
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
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
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


