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
  Serial.print(myData.zData * 9.8, 4);
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

  kxAccel.setRange(0x2);         // 2g Range
  kxAccel.enableDataEngine();     // Enables the bit that indicates data is ready.
  // kxAccel.setOutputDataRate(); // Default is 50Hz
  kxAccel.enableAccel();

}

void loop()
{
  //every 400 millisecconds reads accelerometer(needs to be optimized for low power mode)
  if (lowerZ <= myData.zData <= upperZ) {

    //Motion detection trigger
    bool triggerS = false;//will allow the timestamp when verified movement is detected
    bool triggerE = false;//will allow the timestamp when verified movement is stopped
    bool det = false; // movment detected
    int sampleRate = 400;//sample rate of 400 miliseconds
    float inD = 0.0;
    float upperZ;// upper value of accelerometer rest
    float lowerZ;//lower value of accelerometer rest
    //initial movement detection check
    while (sampleRate != 0) {
      if ( kxAccel.dataReady() )
      {
        kxAccel.getAccelData(&myData);
      }
      Accel();//read accelerometer data
      if (myData.zData >= inD) {
        inD = myData.zData;
        sampleRate--;
        triggerS = true;
      }
      else {//
        triggerS = false;
        sampleRate = 400;
        break;
      }
    }
    if (triggerS == true) {//if movement is detected evi is triggered that triggers the RTC
      digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(20);                       // wait for a second
      digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
      triggerS = false;
      det = true;
    }
    RTC();//reads EVI triggered



    if (det = true) {//movement stop check only triggers if movement was detected
      while (sampleRate != 0) {
        if ( kxAccel.dataReady() )
        {
          kxAccel.getAccelData(&myData);
        }
        Accel();//read accelerometer data
        if (lowerZ <= myData.zData <= upperZ) {
          inD = myData.zData;
          sampleRate--;
          triggerE = true;
        }
        else {//
          triggerE = false;
          break;
        }
      }
      if (triggerE == true) {//if movement is detected evi is triggered that triggers the RTC
        digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(20);                       // wait for a second
        digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
        triggerE = false;
        det = false;//reset detection
      }
      RTC();//reads EVI triggered

    }
  }
  
}//EOF
