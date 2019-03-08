#include <Time.h>
#include <TimeLib.h>
#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Time.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

//instantiate OneWire objects
OneWire oneWire1(2);
OneWire oneWire2(3);
OneWire oneWire3(8);
OneWire oneWire4(5);
OneWire oneWire5(6);
OneWire oneWire6(7);

//instantiate temperature sensors using the OneWire objects
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
DallasTemperature sensor4(&oneWire4);
DallasTemperature sensor5(&oneWire5);
DallasTemperature sensor6(&oneWire6);

 

IPAddress timeServer(130, 159, 248, 123);  //strathclyde time server
const int timeZone = 0;  //Central European Time

byte mac[] = { 0xAA, 0xDE, 0xBE, 0xCF, 0xFE, 0xDD };    //local MAC address, fake
IPAddress localIPNum(172, 16, 1, 21);                   //local ip
byte gatewayIPNum[] = { 172, 16, 0, 1 }; 
byte dnsIPNum[]  = { 8, 8, 8, 8 }; 
byte maskIPNum[] = { 255, 255, 0, 0 }; 


//IP and port for data collection server
IPAddress outIP(172,16,1,10);
unsigned int outPort = 8010;

EthernetUDP Udp;    //instantiate UDP object

/*****************************************************************************
/ general stuff
******************************************************************************/
const int delayTime = 20000;  //in milliseconds


void setup() {
  
  Serial.begin(9600); //9600 baud rate
  delay(250);
  Serial.println("Get NTP timer.");   //debug
  
  Ethernet.begin(mac, localIPNum, dnsIPNum, gatewayIPNum, maskIPNum );      //start ethernet with defined network details
  Udp.begin(8888);                                                          //start UDP server on port 8888

  //debug
  Serial.print("Local IP number: ");    
  Serial.println(Ethernet.localIP() );
  Serial.println("Waiting time sync.");
  setSyncProvider(getNtpTime);  
  setSyncInterval(86400);  

  while (timeStatus() == timeNotSet) {      //until we get a time
    Serial.println("Waiting time sync.");    //print every 3 seconds for debug
    delay(3000);
  }  


  //initialise temperature sensors
  sensor1.begin();
  sensor2.begin();
  sensor3.begin();
  sensor4.begin();
  sensor5.begin();
  sensor6.begin();  
}



void getTempValues(void)
{
  int   i;
  float T1, T2, T3, T4, T5, T6;     //variables for temperature values
  
  String nowStr;                    //string for UNIX time
  String str0,str1m;                //the number of measurements makes a string longer than the buffer will allow, need 2 strings
  
  char UDPBuffer[512];              //buffer to send string with UDP
  
  nowStr = String( now() );         //get time now

  Serial.print("Requesting temperatures...");   //debug
  
  //send the command to get temperatures
  sensor1.requestTemperatures(); 
  sensor2.requestTemperatures(); 
  sensor3.requestTemperatures(); 
  sensor4.requestTemperatures(); 
  sensor5.requestTemperatures(); 
  sensor6.requestTemperatures(); 

  //retrieve temperature values and convert to degC
  T1 = sensor1.getTempCByIndex(0);
  T2 = sensor2.getTempCByIndex(0);
  T3 = sensor3.getTempCByIndex(0);
  T4 = sensor4.getTempCByIndex(0);
  T5 = sensor5.getTempCByIndex(0);
  T6 = sensor6.getTempCByIndex(0);
  
  //debug
  Serial.println("DONE");
  Serial.println(T1); 
  Serial.println(T2); 
  Serial.println(T3); 
  Serial.println(T4); 
  Serial.println(T5); 
  Serial.println(T6); 

  
   //******************************************************
   // send package 1
   //******************************************************
   //populate string with necessary data
   str0 = String("@T-Table_Exp") + "#" + nowStr +  "#" + String( T1 );
   str0 = str0 + String("@T-Room_Exp") + "#" + nowStr +  "#" + String( T2 );
   str0 = str0 + String("@T-RoomAC_Exp_AirOut") + "#" + nowStr +  "#" + String( T3 );
   str0.toCharArray( UDPBuffer, str0.length()+1 );            //convert string to char array
   Serial.println( UDPBuffer );                               //debug
   
   Udp.beginPacket( outIP, outPort );                         //prepare for UDP transfer
   Udp.write(UDPBuffer);                                      //transfer data
   Udp.endPacket();                                           //end transfer
   
   //populate string with necessary data
   str0 = String("@T-TableAC_Exp_AirIn") + "#" + nowStr +  "#" + String( T4 );
   str0 = str0 + String("@T-TableAC_Exp_AirOut") + "#" + nowStr +  "#" + String( T5 );
   str0.toCharArray( UDPBuffer, str0.length()+1 );
   Serial.println( UDPBuffer );
   
   Udp.beginPacket( outIP, outPort );
   Udp.write(UDPBuffer);
   Udp.endPacket();
   
   str0 = String("@T-TableAC_Exp_Water") + "#" + nowStr +  "#" + String( T6 );
   str0.toCharArray( UDPBuffer, str0.length()+1 );
   Serial.println( UDPBuffer );
   
   Udp.beginPacket( outIP, outPort );
   Udp.write(UDPBuffer);
   Udp.endPacket();
}


void loop(void)
{
   getTempValues();       //run temperature collection and UDP transfer
   delay( delayTime );    //delay for 20 s
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


