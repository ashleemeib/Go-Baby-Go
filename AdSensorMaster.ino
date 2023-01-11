//Go Baby Go Adherence Sensor Master Code
//Written by Ashlee B, Kyler M, Tara P
//
//Libraries
#include <SparkFun_RV8803.h> //Get the library here:http://librarymanager/All#SparkFun_RV-8803
#include <Wire.h>
#include "SparkFun_KX13X.h"

//varibles
RV8803 rtc;
SparkFun_KX134 kxAccel;
outputData myData;   // Struct for the accelerometer's data

//Pins
#define EVI 13

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
    while(1);
  }
  Serial.println("RTC online!");
  
  rtc.setEVIEventCapture(RV8803_ENABLE); //Enables the Timestamping function
//Acceleration init
  if( !kxAccel.begin() )
  {
    Serial.println("Could not communicate with the the KX13X. Freezing.");
    while(1);
  }
  
  Serial.println("Ready.");

  if( kxAccel.softwareReset() )
    Serial.println("Reset.");
  kxAccel.enableAccel(false); 

  kxAccel.setRange(0x2);         // 2g Range
  kxAccel.enableDataEngine();     // Enables the bit that indicates data is ready.
  // kxAccel.setOutputDataRate(); // Default is 50Hz
  kxAccel.enableAccel();          

}

void loop()
{
 Accel();
    if(myData.yData*9.8 > 10)
    {
     digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
     delay(20);                       // wait for a second
     digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
     
 RTC();
 
}

void Accel()
{
  // Check if data is ready.
  if( kxAccel.dataReady() )
  {
    kxAccel.getAccelData(&myData); 
    Serial.print("X: ");
    Serial.print(myData.xData*9.8, 4);
    Serial.print(" Y: ");
    Serial.print(myData.yData*9.8, 4);
    Serial.print(" Z: ");
    Serial.print(myData.zData*9.8, 4);
    Serial.println();
   delay(20);
    } 
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

    String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
    String timestamp = rtc.stringTimestamp();

    Serial.print(currentDate);
    Serial.print(" ");
    Serial.println(timestamp);
  }
 }
 
