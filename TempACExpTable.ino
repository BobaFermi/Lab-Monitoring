#include <Time.h>
#include <TimeLib.h>

/*****************************************************************************
* Read temperature from Dallas sensors
* +send to network server with udp
* 
* use uno and network shield
*
* Elmar Haller, Glasgow 2015
*
******************************************************************************/


#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Time.h> 
//#include <Timezone.h>


//United Kingdom (London, Belfast)
//TimeChangeRule rBST = {"BST", Last, Sun, Mar, 1, 60};        //British Summer Time
//TimeChangeRule rGMT = {"GMT", Last, Sun, Oct, 2, 0};         //Standard Time
//Timezone UK(rBST, rGMT);

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire1(2);
OneWire oneWire2(3);
OneWire oneWire3(8);
OneWire oneWire4(5);
OneWire oneWire5(6);
OneWire oneWire6(7);

DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);
DallasTemperature sensor3(&oneWire3);
DallasTemperature sensor4(&oneWire4);
DallasTemperature sensor5(&oneWire5);
DallasTemperature sensor6(&oneWire6);

/************************************************************
// network stuff
/***********************************************************/

// NTP Servers:
IPAddress timeServer(130, 159, 248, 123);  // strathclyde
const int timeZone = 0;  // Central European Time

// local MAC address, fake
byte mac[] = { 0xAA, 0xDE, 0xBE, 0xCF, 0xFE, 0xDD };
// local ip
IPAddress localIPNum(172, 16, 1, 21);
byte gatewayIPNum[] = { 172, 16, 0, 1 }; 
byte dnsIPNum[]  = { 8, 8, 8, 8 }; 
byte maskIPNum[] = { 255, 255, 0, 0 }; 


// data collection server
IPAddress outIP(172,16,1,10);
unsigned int outPort = 8010;

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

/*****************************************************************************
/ general stuff
******************************************************************************/
const int delayTime = 20000;  // in milliseconds


/* send with netcat in linux: echo -n "@YUN#3335.4e19#234.11" | netcat -c -w1 -u 192.168.100.1 8010 */
//String s1   = "echo -n \"@";
//String s2   = "\" | netcat -c -w1 -u 172.16.0.1 8010 ";



void setup() {
  
  Serial.begin(9600);
  delay(250);
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


  // Start up the temperature library
  sensor1.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  sensor2.begin();
  sensor3.begin();
  sensor4.begin();
  sensor5.begin();
  sensor6.begin();  
}



void getTempValues(void)
{
  int   i;
  float T1, T2, T3, T4, T5, T6;
  
  String nowStr;
  String str0,str1m;  
  
  // buffer to send string with UDP
  char UDPBuffer[512];          

  
  // get time now
  nowStr = String( now() );
    
  //******************************************************
  // get temperature values
  //******************************************************
  Serial.print("Requesting temperatures...");
  sensor1.requestTemperatures(); // Send the command to get temperatures
  sensor2.requestTemperatures(); // Send the command to get temperatures  
  sensor3.requestTemperatures(); // Send the command to get temperatures  
  sensor4.requestTemperatures(); // Send the command to get temperatures 
  sensor5.requestTemperatures(); // Send the command to get temperatures   
  sensor6.requestTemperatures(); // Send the command to get temperatures 

  T1 = sensor1.getTempCByIndex(0);
  T2 = sensor2.getTempCByIndex(0);
  T3 = sensor3.getTempCByIndex(0);
  T4 = sensor4.getTempCByIndex(0);
  T5 = sensor5.getTempCByIndex(0);
  T6 = sensor6.getTempCByIndex(0);
  
  Serial.println("DONE");
  Serial.println(T1); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.println(T2); 
  Serial.println(T3); 
  Serial.println(T4); 
  Serial.println(T5); 
  Serial.println(T6); 

  
   //******************************************************
   // send package 1
   //******************************************************
   str0 = String("@T-Table_Exp") + "#" + nowStr +  "#" + String( T1 );
   str0 = str0 + String("@T-Room_Exp") + "#" + nowStr +  "#" + String( T2 );
   str0 = str0 + String("@T-RoomAC_Exp_AirOut") + "#" + nowStr +  "#" + String( T3 );
   str0.toCharArray( UDPBuffer, str0.length()+1 );
   Serial.println( UDPBuffer );
   
   Udp.beginPacket( outIP, outPort );
   Udp.write(UDPBuffer);
   Udp.endPacket();
   
   
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
   getTempValues();
   delay( delayTime );
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


