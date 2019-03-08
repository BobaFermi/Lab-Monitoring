/*****************************************************************************
* Read analog inputs
* (1) read analog 
* (2) send to Linino
* (3) run server on linux side?
*
* The timerOne library does not work yet with the YUN
* I removed the timer, and used a loose timing
*
* Elmar Haller, Glasgow 2014
*
******************************************************************************/
#include <Console.h>
#include <Process.h>


/*****************************************************************************
/ Temperature
/ for dallas temperature sensors 
******************************************************************************/

#include <OneWire.h>            
#include <DallasTemperature.h>
#include<Wire.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire1(2);
OneWire oneWire2(3);
OneWire oneWire3(4);
OneWire oneWire4(5);

OneWire oneWire5(6);
OneWire oneWire6(7);
OneWire oneWire7(8);
OneWire oneWire8(9);

OneWire oneWire9(10);


// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensorT1(&oneWire1);
DallasTemperature sensorT2(&oneWire2);
DallasTemperature sensorT3(&oneWire3);
DallasTemperature sensorT4(&oneWire4);

DallasTemperature sensorT5(&oneWire5);
DallasTemperature sensorT6(&oneWire6);
DallasTemperature sensorT7(&oneWire7);
DallasTemperature sensorT8(&oneWire8);

DallasTemperature sensorT9(&oneWire9);



/*****************************************************************************
/ general stuff
******************************************************************************/
Process pGetDate;              // process used to get the date
Process pSendData;             // process used to get the date

const int pinLED    = 13;  // the pin with a LED
const int delayTime = 3000;  // in milliseconds


/* send with netcat in linux: echo -n "@YUN#3335.4e19#234.11" | netcat -c -w1 -u 192.168.100.1 8010 */
String s1   = "echo -n \"@";
String s2   = "\" | netcat -c -w1 -u 172.16.1.10 8010 ";

String timeString;



void setup(void)
{
  // initialize serial communication:
  Bridge.begin();
  Console.begin(); 
  
 
  /*************************************************
  // OneWire library
  /*************************************************/
  Console.println("Connecting to temperature sensors");
  // Start up the library
  sensorT1.begin(); 
  sensorT2.begin(); 
  sensorT3.begin(); 
  sensorT4.begin(); 

  sensorT5.begin(); 
  sensorT6.begin(); 
  sensorT7.begin(); 
  sensorT8.begin(); 

  sensorT9.begin(); 
}


void getAnalogValues(void)
{
  String sendStr0;
  
  // test for total duration
  digitalWrite(pinLED, HIGH); 


  //******************************************************
  // get temperatures
  //******************************************************
  timeString = getTimeStamp();     

  //Console.print("Requesting temperatures...");
  sensorT1.requestTemperatures(); // Send the command to get temperatures  
  sensorT2.requestTemperatures();
  sensorT3.requestTemperatures();
  sensorT4.requestTemperatures();
  
  sensorT5.requestTemperatures();
  sensorT6.requestTemperatures();
  sensorT7.requestTemperatures();
  sensorT8.requestTemperatures();

  sensorT9.requestTemperatures();

  
  float T1 = sensorT1.getTempCByIndex(0);
  float T2 = sensorT2.getTempCByIndex(0);
  float T3 = sensorT3.getTempCByIndex(0);
  float T4 = sensorT4.getTempCByIndex(0);

  float T5 = sensorT5.getTempCByIndex(0);
  float T6 = sensorT6.getTempCByIndex(0);
  float T7 = sensorT7.getTempCByIndex(0);
  float T8 = sensorT8.getTempCByIndex(0);
  
  float T9 = sensorT9.getTempCByIndex(0);
  
  sendStr0 = String( s1 + "T-LaserTable_1" + "#" + timeString + "#" + String(T1));
  sendStr0 = sendStr0 + "@" + "T-TableAC_Laser_AirIn" + "#" + timeString + "#" + String(T2) + 
                        "@" + "T-TableAC_Laser_AirOut" + "#" + timeString + "#" + String(T3);
  sendStr0 = sendStr0 + "@" + "T-TableAC_Laser_Water" + "#" + timeString + "#" + String(T4);
  sendStr0 = sendStr0 + s2; 
  pSendData.runShellCommand( sendStr0 );           
  Console.println( sendStr0 );  
  
  sendStr0 = String( s1 + "T-RoomAC_Laser_AirOut" + "#" + timeString + "#" + String(T5));
  sendStr0 = sendStr0 + "@" + "T-Room_Laser_(Door)" + "#" + timeString + "#" + String(T6) + 
                        "@" + "T-Room_Laser_(Table)" + "#" + timeString + "#" + String(T7);
  sendStr0 = sendStr0 + "@" + "T-Room_Laser_(Mid)" + "#" + timeString + "#" + String(T8) + 
                        "@" + "T-LaserTable_2" + "#" + timeString + "#" + String(T9);
  sendStr0 = sendStr0 + s2; 
  pSendData.runShellCommand( sendStr0 );           
  Console.println( sendStr0 );  

   
  // test for total duration
  digitalWrite(pinLED, LOW); 
}


void loop(void)
{
   getAnalogValues();
   delay( delayTime );
}



/**<--------------------     support function    --------------------->*/
String getTimeStamp() {
  String result;
  
  if (!pGetDate.running())  {
    pGetDate.begin("date");
    pGetDate.addParameter("+%s");
    pGetDate.run();
  }  

  while(pGetDate.available()>0) {
    char c = pGetDate.read();
    if(c != '\n')
      result += c;
  }
  result.trim();
  return result;
}
