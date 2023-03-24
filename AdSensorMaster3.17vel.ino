//Go Baby Go Adherence Sensor Master Code
//Written by Ashlee B, Kyler M, Tara P
//
//Components:
//Accelerometer- reading implemented
//Low Power Mode
//RTC- EVI Pin implemented
//SD Card R/W
//Bluetooth(possible)
//
//Libraries
#include <Wire.h>
#include <SparkFun_RV8803.h>
#include "SparkFun_KX13X.h"

//varibles
RV8803 rtc;
SparkFun_KX134 kxAccel;
outputData myData;   // Struct for the accelerometer's data
float  zO = 0.0;//zero g offset
float zOx = 0.0; // x direction offset
//float zOy = 0.0;
//float zOz = 0.0;
float lastVel = 0.0;
float vel;
float curVel = 0.0;
float accel;
float lastAccel = 0.0;
//int det = 0;
//int det1 = 0;
int timet = 100;
bool detM = false;//detection movement
//bool detS = false;//detection stop
bool set = false;

//Pins
#define EVI 13//timestamp 

//timestamp subroutine
void RTC()
{
  //RTC timestamp
  if (rtc.getInterruptFlag(FLAG_EVI))
  {
    rtc.updateTime();
    rtc.clearInterruptFlag(FLAG_EVI);
    // debugging
    String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
    String timestamp = rtc.stringTimestamp();

    Serial.print(currentDate);
    Serial.print(" ");
    Serial.println(timestamp);
    //end debug
  }
}
//reset function
void(* resetFunc) (void) = 0; //declare reset function @ address 0
//initialize
void setFunc() {
  //check if zero g offest records new data
  while (timet != 0) {
    kxAccel.getAccelData(&myData);//grab new acceleration data each rep
    Serial.print("X1: ");
    Serial.print(myData.xData * 9.80665, 4);//measured in g's then converted to m/s^2
    Serial.println();

    zOx = (myData.xData * 9.80665) + zOx;
    timet --;
  }
  Serial.print("SET");
  timet = 100;
  set = true;
  zO = (zOx) / 100;//zero g axis of x axis
  Serial.print(zO);
}

}
void setup()
{
  pinMode(EVI, OUTPUT);
  Wire.begin();//I2C addresses begin
  Serial.begin(115200);//set baud rate
  //RTC initialization
  if (rtc.begin() == false)
  {
    Serial.println("Device not found. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("RTC online!");

  rtc.setEVIEventCapture(RV8803_ENABLE); //Enables the Timestamping function
  //Acceleration init
  if ( !kxAccel.begin() )
  {
    Serial.println("Could not communicate with the the KX13X. Freezing.");
    while (1);
  }
  Serial.println("Ready.");

  if ( kxAccel.softwareReset() )
    Serial.println("Reset.");
  kxAccel.enableAccel(false);

  kxAccel.setRange(0x02);         // 2g Range
  kxAccel.enableDataEngine();     // Enables the bit that indicates data is ready.
  //kxAccel.setOutputDataRate(); // Default is 50Hz
  kxAccel.enableAccel();

}

void loop()
{

  // Check if data is ready.
  if ( kxAccel.dataReady() )
  {
    kxAccel.getAccelData(&myData);
    if ( set == false) {
      setFunc();//calculates the zerog offset
    }
  } else {
    //resetFunc();
    Serial.print("data not ready");
  }

  //constants calculations
  accel = (myData.xData * 9.80665) - zO;
  vel =  /*lastVel +*/ ((accel /*- lastAccel)*/ * 0.1); //Possibly needs reworking - a*t
                        curVel = curVel + vel;//current velocity update
                        // detection from idle
  if (abs(curVel) >= 0.05 && detM == false) {//absolute value to account for positive and negative change
  Serial.print("V:");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Detected ");//Debug purposes
    RTC();//timestamp
    //insert print to SD code here
    detM = true;
  } else if (abs(curVel) < 0.04 && detM == true) {//sto detection, parameters need testing
  Serial.print(" V: ");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Stopped");
    RTC();
    detM = false;
  } else {
    Serial.print(" cV: ");
    Serial.print(curVel, 8);
    Serial.print(" V: ");
    //Serial.print(vel, 8);
    Serial.println();
    Serial.print("L A: ");
    // Serial.print((accel), 4);
    Serial.print(" lA: ");
    //Serial.print((lastAccel), 4);
    Serial.println();
  }
  //sets last values
  lastVel = vel;
            lastAccel = accel;

            delay(100); // (Delay should be 1/ODR (Output Data Rate), default was 1/50ODR(adjusted)50hz) now is 10Hz



}//EOF
