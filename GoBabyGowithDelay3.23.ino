//Go Baby Go Adherence Sensor Master Code
//Written by Ashlee B, Kyler M, Tara P
//
//Components:
//Accelerometer- reading implemented
//Low Power Mode
//RTC- EVI Pin implemented
//SD Card R/W
//Bluetooth(if possible)

//Libraries
#include <Wire.h>
#include <SparkFun_RV8803.h>
#include "SparkFun_KX13X.h"

//variables
RV8803 rtc;
SparkFun_KX134 kxAccel;
outputData myData;   // Struct for the accelerometer's data

const float gravity = 9.80665;  // earth's gravity in m/s^2
float zO = 0.0; // initialize zero g offset to 0.0
float zOx = 0.0; // initialize x direction offset to 0.0
float lastVel = 0.0; // initialize last velocity value
float vel; 
float curVel = 0.0; // initialize current velocity value
float accel;
float lastAccel = 0.0; // initialize last acceleration value
int timet = 100; // samples for acceleration data
bool detM = false; // wheelchair moved
bool detS = false; // wheelchair stopped
bool set = false; // calculate zero-g offset

//Pins
#define EVI 13

//RTC timestamp subroutine
void RTC() {
  if (rtc.getInterruptFlag(FLAG_EVI)) {
    rtc.updateTime();
    rtc.clearInterruptFlag(FLAG_EVI);
    
    // debugging
    String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format
    String timestamp = rtc.stringTimestamp();

    Serial.print(currentDate);
    Serial.print(" ");
    Serial.println(timestamp);
    //end debug
  }
}

// Reset function
void(* resetFunc) (void) = 0; //declare reset function @ address 0

// Determines the value for zero-g offset. Collects 100 acceleration data zOx points and determines the average value for zO
void setFunc(){
  while (timet > 0) {
    Serial.print("Z1: ");
    Serial.print(myData.xData * gravity, 4);
    Serial.println();
    zOx = (myData.xData * gravity) + zOx;
    timet --;
  }
  Serial.print("SET");
  timet = 100;
  set = true; 
  zO = (zOx) / timet;
  Serial.print(zO);
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
  if ( kxAccel.dataReady() ) {
    kxAccel.getAccelData(&myData);
    if (set == false) {
     setFunc(); // calculates the zero-g offset
    }
  } 
  else {
    //resetFunc();
  }
  
  // constants calculations
  accel = (myData.xData * gravity) - zO;
  vel = (accel *(.03333)); // calculate change in velocity (measured acceleration multiplied by change in time)
  curVel = curVel + vel; // current velocity update (previous velocity plus change in velocity

  // detection from idle
  if (abs(curVel) > 0.04 && detM == false) { //absolute value to account for positive and negative change
    Serial.print("V:");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20); // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Detected ");//Debug purposes
    RTC();//timestamp
    //insert print to SD code here
    detM = true;
  } 
  else if (abs(curVel) <= 0.04 && detM == true) { //stop detection, parameters need testing
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
    Serial.print(vel, 8);
    Serial.println();
    Serial.print("L A: ");
    Serial.print((accel), 4);
    Serial.print(" lA: ");
    Serial.print((lastAccel), 4);
    Serial.println();
  }

  delay(33.33); // (Delay should be 1/ODR (Output Data Rate), default was 1/50ODR(adjusted)50hz) now is 30Hz

}

//EOF
