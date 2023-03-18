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
float  zO = 0.0;
float zOx = 0.0;
//float zOy = 0.0;
//float zOz = 0.0;
float lastVel = 0.0;
float vel;
float curVel = 0.0;
float accel;
float lastAccel = 0.0;
int det = 0;
int det1 = 0;
int timet = 100;
int sampRate = 50;
bool detM = false;
bool detS = false;
bool set = false;

//Pins
#define EVI 13
//Accelerometer subroutine:(Debug:serial prints the readings)
void Accel()
{
  // Check if data is ready.


  Serial.print("X: ");
  Serial.print(myData.xData * 9.8, 4);
  Serial.print(" Y: ");
  Serial.print(myData.yData * 9.8, 4);
  Serial.print(" Z: ");
  Serial.print(myData.xData * 9.8, 4);
  Serial.println();

  //debug(may comment out delay)
  delay(20); // Delay should be 1/ODR (Output Data Rate), default is 1/50ODR


}
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
      while (timet != 0) {
        Serial.print("Z1: ");
        Serial.print(myData.xData * 9.80665, 4);
        Serial.println();

        zOx = (myData.xData * 9.80665) + zOx;
        timet --;
      }
      Serial.print("SET");
      timet = 100;
      set = true;
      zO = (zOx) / 100;
      Serial.print(zO);
    }
  } else {
    //resetFunc();
  }
  //constants calculations
  accel = (myData.xData * 9.80665) - zO;
  vel =  lastVel + (accel - lastAccel) * (1 / 10);
  curVel = curVel + vel;
  // detection from idle
  if (curVel >= 0.1 && detM == false) {
    Serial.print("V:");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
    Serial.print("Detected ");
    RTC();//timestamp
    //print to SD
    // det1 = 0;//reset
    detM = true;
  } else if (curVel < 0.1 && detM == true) {
    Serial.print(" V: ");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
    Serial.print("Stopped");
    RTC();
    detM = false;
  } else {
    Serial.print(" lV: ");
    Serial.print(lastVel, 8);
    Serial.print(" V: ");
    Serial.print(vel, 8);
    Serial.println();
    Serial.print(" A: ");
    Serial.print((accel), 4);
    Serial.print(" lA: ");
    Serial.print((lastAccel), 4);
    Serial.println();
  }
  lastVel = vel;
  lastAccel = accel;

  delay(100); // Delay should be 1/ODR (Output Data Rate), default is 1/50ODR(adjusted)



}//EOF
