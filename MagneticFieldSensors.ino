#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <Time.h> 


#include <Wire.h>
#include <SoftwareWire.h>

#define HMC5883L  0x1E 


SoftwareWire magSensor0( 40, 41);
SoftwareWire magSensor1( 42, 43);
SoftwareWire magSensor2( 44, 45);
SoftwareWire magSensor3( 46, 47);

int mag0_X, mag0_Y, mag0_Z;
int mag1_X, mag1_Y, mag1_Z;
int mag2_X, mag2_Y, mag2_Z;
int mag3_X, mag3_Y, mag3_Z;

/************************************************************
// network stuff
/***********************************************************/

// NTP Servers:
IPAddress timeServer(130, 159, 248, 123);  // strathclyde==
const int timeZone = 0;  // Central European Time

// local MAC address, fake
byte mac[] = { 0xAA, 0xDE, 0xBE, 0xCF, 0xFE, 0xDD };
// local ip
IPAddress localIPNum(172, 16, 1, 17);
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
const long delayTime = 40000L;  // in milliseconds

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


  /*************************************************
  * initialise/configure magetic field sensors
  *************************************************/
  magSensor0.begin(); 
  magSensor0.beginTransmission(HMC5883L);
  magSensor0.write( byte(0x00) ); //go to configuration register A (Averaged samples and output rates)
  magSensor0.write( byte(0x60) ); //average over 8 samples, set data output rate to 0.75Hz
  magSensor0.endTransmission();
  magSensor0.beginTransmission(HMC5883L);
  magSensor0.write( byte(0x01) ); //go to configuration register B (gain)
  magSensor0.write( byte(0x00) ); //gain setting (highest -> lowest: 00, 20, 40, 60, 80, A0, C0, E0)
  magSensor0.endTransmission();
  magSensor0.beginTransmission(HMC5883L);
  magSensor0.write( byte(0x02) ); //go to mode register (to select operation mode)
  magSensor0.write( byte(0x00) ); //select continuous measurement mode
  magSensor0.endTransmission(HMC5883L);  
  mag0_X = 0; mag0_Y = 0; mag0_Z = 0;    
  magSensor1.begin(); 
  magSensor1.beginTransmission(HMC5883L);
  magSensor1.write( byte(0x00) ); //go to configuration register A (Averaged samples and output rates)
  magSensor1.write( byte(0x60) ); //average over 8 samples, set data output rate to 0.75Hz
  magSensor1.endTransmission();
  magSensor1.beginTransmission(HMC5883L);
  magSensor1.write( byte(0x01) ); //go to configuration register B (gain)
  magSensor1.write( byte(0x00) ); //gain setting (highest -> lowest: 00, 20, 40, 60, 80, A0, C0, E0)
  magSensor1.endTransmission();
  magSensor1.beginTransmission(HMC5883L);
  magSensor1.write( byte(0x02) ); //go to mode register (to select operation mode)
  magSensor1.write( byte(0x00) ); //select continuous measurement mode
  magSensor1.endTransmission(HMC5883L);  
  mag1_X = 0; mag1_Y = 0; mag1_Z = 0;    
  magSensor2.begin(); 
  magSensor2.beginTransmission(HMC5883L);
  magSensor2.write( byte(0x00) ); //go to configuration register A (Averaged samples and output rates)
  magSensor2.write( byte(0x60) ); //average over 8 samples, set data output rate to 0.75Hz
  magSensor2.endTransmission();
  magSensor2.beginTransmission(HMC5883L);
  magSensor2.write( byte(0x01) ); //go to configuration register B (gain)
  magSensor2.write( byte(0x00) ); //gain setting (highest -> lowest: 00, 20, 40, 60, 80, A0, C0, E0)
  magSensor2.endTransmission();
  magSensor2.beginTransmission(HMC5883L);
  magSensor2.write( byte(0x02) ); //go to mode register (to select operation mode)
  magSensor2.write( byte(0x00) ); //select continuous measurement mode
  magSensor2.endTransmission(HMC5883L);  
  mag2_X = 0; mag2_Y = 0; mag2_Z = 0;  
  magSensor3.begin(); 
  magSensor3.beginTransmission(HMC5883L);
  magSensor3.write( byte(0x00) ); //go to configuration register A (Averaged samples and output rates)
  magSensor3.write( byte(0x60) ); //average over 8 samples, set data output rate to 0.75Hz
  magSensor3.endTransmission();
  magSensor3.beginTransmission(HMC5883L);
  magSensor3.write( byte(0x01) ); //go to configuration register B (gain)
  magSensor3.write( byte(0x20) ); //gain setting (highest -> lowest: 00, 20, 40, 60, 80, A0, C0, E0)
  magSensor3.endTransmission();
  magSensor3.beginTransmission(HMC5883L);
  magSensor3.write( byte(0x02) ); //go to mode register (to select operation mode)
  magSensor3.write( byte(0x00) ); //select continuous measurement mode
  magSensor3.endTransmission(HMC5883L);  
  mag3_X = 0; mag3_Y = 0; mag3_Z = 0;  
}

int x,y,z;
int xa,ya,za;
int xb,yb,zb;
int xc,yc,zc;
int mGmax;
int mGmaxneg;
int mGmax1;
int mGmaxneg1;
float mGconvert;
float mGconvert1;
float mGx,mGy,mGz;
float mGxa,mGya,mGza;
float mGxb,mGyb,mGzb;
float mGxc,mGyc,mGzc;

void getMagValues(void)
{
  int   i;
  float T1;
  
  String nowStr;
  String str0,str1m;  
  
  // buffer to send string with UDP
  char UDPBuffer[512];          

  
  // get time now
  nowStr = String( now() );
  
  
  // test for total duration
  digitalWrite( 13, HIGH); 
  
  //******************************************************
  // get magnetic field values
  //******************************************************
  Serial.print("Requesting magnetic field...");
    magSensor0.beginTransmission(HMC5883L);
  magSensor0.write(0x03);
  magSensor0.endTransmission();
  magSensor0.requestFrom(HMC5883L,6);
  y = magSensor0.read() << 8;
  y |= magSensor0.read();
  z = magSensor0.read() << 8;
  z |= magSensor0.read();
  x = magSensor0.read() << 8;
  x |= magSensor0.read(); 
  magSensor1.beginTransmission(HMC5883L);
  magSensor1.write(0x03);
  magSensor1.endTransmission();
  magSensor1.requestFrom(HMC5883L,6);
  xa = magSensor1.read() << 8;
  xa |= magSensor1.read();
  ya = magSensor1.read() << 8;
  ya |= magSensor1.read();
  za = magSensor1.read() << 8;
  za |= magSensor1.read(); 
  magSensor2.beginTransmission(HMC5883L);
  magSensor2.write(0x03);
  magSensor2.endTransmission();
  magSensor2.requestFrom(HMC5883L,6);
  yb = magSensor2.read() << 8;
  yb |= magSensor2.read();
  zb = magSensor2.read() << 8;
  zb |= magSensor2.read();
  xb = magSensor2.read() << 8;
  xb |= magSensor2.read(); 
  magSensor3.beginTransmission(HMC5883L);
  magSensor3.write(0x03);
  magSensor3.endTransmission();
  magSensor3.requestFrom(HMC5883L,6);
  yc = magSensor3.read() << 8;
  yc |= magSensor3.read();
  zc = magSensor3.read() << 8;
  zc |= magSensor3.read();
  xc = magSensor3.read() << 8;
  xc |= magSensor3.read(); 
  
//******************************************************
// convert retrieved values to milliGauss
//******************************************************

mGmax = 880.0; //Depends on selected gain (highest -> lowest: 880.0 1300.0 1900.0 2500.0 4000.0 4700.0 5600.0 8100.0)
mGmaxneg = -880.0;
mGmax1 = 1300.0;
mGmaxneg1 = -1300.0;
mGconvert = mGmax / 2048.0;
mGconvert1 = mGmax1 / 2048.0;
mGx = x * mGconvert;
mGy = y * mGconvert;
mGz = z * mGconvert;
mGxa = xa * mGconvert;
mGya = ya * mGconvert;
mGza = za * mGconvert;
mGxb = -1 * xb * mGconvert;
mGyb = yb * mGconvert;
mGzb = -1 * zb * mGconvert;
mGxc = -1 * xc * mGconvert1;
mGyc = yc * mGconvert1;
mGzc = -1 * zc * mGconvert1;
  if ( (mGx < mGmax) && (mGx >= -1760)) {
    str0 = str0 + String("@B_DoorTop_x") + "#" + nowStr +  "#" + String( mGx ); 
  }
  
  if ( (mGy < mGmax) && (mGy >= -1760)) {
    str0 = str0 + String("@B_DoorTop_y") + "#" + nowStr +  "#" + String( mGy ); 
  }
  
  if ( (mGz < mGmax) && (mGz >= -1760)) {
    str0 = str0 + String("@B_DoorTop_z") + "#" + nowStr +  "#" + String( mGz ); 
  }
  
  if ( (mGxa < mGmax) && (mGxa >= -1760)) {
    str0 = str0 + String("@B_DoorBottom_x") + "#" + nowStr +  "#" + String( mGxa ); 
  }
  
  if ( (mGya < mGmax) && (mGya >= -1760)) {
    str0 = str0 + String("@B_DoorBottom_y") + "#" + nowStr +  "#" + String( mGya ); 
  }
  
  if ( (mGza < mGmax) && (mGza >= -1760)) {
    str0 = str0 + String("@B_DoorBottom_z") + "#" + nowStr +  "#" + String( mGza ); 
  }
  
  if ( (mGxb > mGmaxneg) && (mGxb <= 1760)) {
    str0 = str0 + String("@B_CurtainTop_x") + "#" + nowStr +  "#" + String( mGxb ); 
  
  
  if ( (mGyb < mGmax) && (mGyb >= -1760)) {
    str0 = str0 + String("@B_CurtainTop_y") + "#" + nowStr +  "#" + String( mGyb ); 
  }
  
  if ( (mGzb > mGmaxneg) && (mGzb <= 1760)) {
    str0 = str0 + String("@B_CurtainTop_z") + "#" + nowStr +  "#" + String( mGzb ); 
  }
  
  if ( (mGxc > mGmaxneg1) && (mGxc <= 1760)) {
    str0 = str0 + String("@B_CurtainBottom_x") + "#" + nowStr +  "#" + String( mGxc ); 
  }
  
  if ( (mGyc < mGmax1) && (mGyc >= -1760)) {
    str0 = str0 + String("@B_CurtainBottom_y") + "#" + nowStr +  "#" + String( mGyc ); 
  }
  
  if ( (mGzc > mGmaxneg1) && (mGzc <= 1760)) {
    str0 = str0 + String("@B_CurtainBottom_z") + "#" + nowStr +  "#" + String( mGzc ); 
  }
  
  Serial.println("DONE");
  Serial.print( "DoorTop x = " ); Serial.println(mGx);
  Serial.print( "DoorTop y = " ); Serial.println(mGy);
  Serial.print( "DoorTop z = " ); Serial.println(mGz); 
  Serial.print( "DoorBottom x = " ); Serial.println(mGxa);
  Serial.print( "DoorBottom y = " ); Serial.println(mGya);
  Serial.print( "DoorBottom z = " ); Serial.println(mGza);   
  Serial.print( "CurtainTop x = " ); Serial.println(mGxb);
  Serial.print( "CurtainTop y = " ); Serial.println(mGyb);
  Serial.print( "CurtainTop z = " ); Serial.println(mGzb); 
  Serial.print( "CurtainBottom x = " ); Serial.println(mGxc);
  Serial.print( "CurtainBottom y = " ); Serial.println(mGyc);
  Serial.print( "CurtainBottom z = " ); Serial.println(mGzc);   
  
}
   //******************************************************
   // send package 
   //******************************************************

   str0.toCharArray( UDPBuffer, str0.length()+1 );
   Serial.println( UDPBuffer );
   
   Udp.beginPacket( outIP, outPort );
   Udp.write(UDPBuffer);
   Udp.endPacket();

   // test for total duration
   digitalWrite( 13, LOW); 
}


void loop(void)
{
   getMagValues();
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


