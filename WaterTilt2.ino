/*****************************************************************************
* Get Water sensors and tilt values
*
******************************************************************************/

#include "Wire.h"
#include <Math.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <SPI.h> 

MPU6050 accel;

unsigned long time;

const float Pi = 3.141592653589;
int16_t ax, ay, az;
long totx;
long toty;
long totz;

int avex;
int avey;
int avez;

float anglexz;
float angleyz;

String sendStr0;
String nowstr;

void setup() {
  
    Wire.begin();
    Wire.write( byte(0x1c) );
    Wire.write( byte(0x00) );

    //Serial.begin(9600);
    Serial.begin(57600);
    
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    
    accel.initialize();
    accel.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    
}

void loop() {
  /******************************
  * get acceleration
  *****************************/
  totx = 0;
  toty = 0;
  totz = 0;
 // for (int i1=0; i1<50; i1++) {
     accel.getAcceleration(&ax, &ay, &az);
  
     
 //    totx = totx + ax;
 //    toty = toty + ay;
 //    totz = totz + az;
 // }      
  time = millis();
 // avex = (float) (totx/50);        
 // avey = (float) (toty/50);        
 // avez = (float) (totz/50);
  avex = ax;
  avey = ay;
  avez = az;

  //anglexz =  180/Pi *acos ( float(ax)/float(az) );
  //angleyz = 180/Pi * acos ( float(ay)/float(az) );
  
  //Serial.print(anglexz,8); Serial.print("\t");
  //Serial.print(angleyz,8); Serial.print("\t");
      

  Serial.print("@");
  Serial.print(avex);
  Serial.print("#");
  Serial.print(avey);
  Serial.print("#");
  Serial.print(avez);
  Serial.print("#");
  Serial.print(time);  
  Serial.println("E");
  

  //anglexz = 180/Pi * (acos ( float(avex)/float(avez) ) - (Pi/2));
  //angleyz = 180/Pi * (acos ( float(avey)/float(avez) ) - (Pi/2));

  
//   Console.print("x-z angle: "); Console.print(anglexz); Console.print("\t"); Console.print("\t");
//   Console.print("y-z angle: "); Console.println(angleyz);
    
     delay(50);
}
