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
float  mx = 0.0;
float mTx = 0.0;
float mTy = 0.0;
float mTz = 0.0;
float mT = 0.195026;
int det = 0;
float last = 0.0;
int timet = 100;
bool detb = false;
bool set = false;
int sampRate = 50;
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
        Serial.print(myData.zData * 9.80665, 4);
        Serial.println();

        mTx = (myData.zData * 9.80665) + mTx;
        timet --;
      }
      Serial.print("SET");
      timet = 100;
      set = true;
      mT = (mTx) / 100;
      Serial.print(mT);
    }
  } else{
    //resetFunc();
  }
  // detection from idle
  if (((myData.zData * 9.80665)) - mT  > mT ||((myData.zData * 9.80665)) - mT  < -mT  && detb == false) {
    Serial.print(" Z0: ");
    Serial.print(((myData.zData * 9.80665) - mT), 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
    Serial.print("Detected ");
    RTC();//timestamp
    det = 0;//reset
    detb = true;


    //checking over 50 ms
  }/* else if (abs((myData.zData * 9.80665) - mT) > mT && detb == true) {
      Serial.print(" Z0: ");
      Serial.print(abs((myData.zData * 9.80665) - mT), 4);
      Serial.println();
      if (abs((myData.zData * 9.80665) - mT) > last) {
        det += 1;
      }
    }*/
  //first detects stop
  else if (((myData.zData * 9.80665) - mT) <= mT ||((myData.zData * 9.80665)) - mT  >= -mT && det < sampRate && detb == true) {
    Serial.print(" Z1: ");
    Serial.print(((myData.zData * 9.80665) - mT), 4);
    Serial.println();
    if ( last + last / 2 > (myData.zData * 9.80665) - mT) {
      det = 0;
    } else {
      det += 1;
    }
  } else if (((myData.zData * 9.80665) - mT) <= mT ||((myData.zData * 9.80665)) - mT  >= -mT && det == sampRate && detb == true) {
    Serial.print(" Z2: ");
    Serial.print(((myData.zData * 9.80665) - mT), 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn the LED off by making the voltage LOW
    Serial.print("Stopped");
    RTC();
    det = 0;
    detb = false;
  } else {
    Serial.print(" Z: ");
    Serial.print(((myData.zData * 9.80665) - mT), 4);
    Serial.println();
  }

  delay(50); // Delay should be 1/ODR (Output Data Rate), default is 1/50ODR(adjusted)



}//EOF
