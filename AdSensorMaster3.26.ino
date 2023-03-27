
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
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//variables
RV8803 rtc;
SparkFun_KX134 kxAccel;
outputData myData;   // Struct for the accelerometer's data

const float gravity = 9.80665;  // earth's gravity in m/s^2
float  zO = 0.0; // zero g offset
float zOx = 0.0; // x direction offset
//float zOy = 0.0;
//float zOz = 0.0;
float lastVel = 0.0; // initialize last velocity value
float vel;
float curVel = 0.0; // initialize current velocity value
float accel;
float lastAccel = 0.0; // initialize last acceleration value
//int det = 0;
//int det1 = 0;
int timet = 100; // samples for acceleration data
bool detM = false; // detection movement
bool calc = false; // calculate velocity
bool set = false;

//Pins
#define EVI 13 // timestamp 

//timestamp subroutine
String RTC()
{
  String time;
  
  //RTC timestamp
  if (rtc.getInterruptFlag(FLAG_EVI)) {
    rtc.updateTime();
    rtc.clearInterruptFlag(FLAG_EVI);
    
    // Get the current date in mm/dd/yyyy format
    String currentTime = rtc.stringDateUSA(); 

    //Get the timestamp
    String timestamp = rtc.stringTimestamp();

    time = currentTime + " " + timestamp;
    
    Serial.print(time);
    //end debug
  }
  
  return time;
}

//reset function
void(* resetFunc) (void) = 0; //declare reset function @ address 0

//initialize
void setFunc() {
  // check if zero g offest records new data
  while (timet > 0) {
    kxAccel.getAccelData(&myData); // grab new acceleration data each rep
    Serial.print("X1: ");
    Serial.print(myData.xData * gravity, 4); // measured in g's then converted to m/s^2
    Serial.println();

    zOx = (myData.xData * gravity) + zOx;
    timet --;
    delay(33.33);
  }
  Serial.print("SET");
  timet = 100;
  set = true;
  zO = (zOx) / 100; // zero g axis of x axis
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
  kxAccel.setOutputDataRate(30); // Default is 50Hz //set at 30 Hz
  kxAccel.enableAccel();

  // Check if SD is connected correctly
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
     Serial.println("MMC");
  } 
  else if(cardType == CARD_SD){
     Serial.println("SDSC");
  } 
  else if(cardType == CARD_SDHC){
     Serial.println("SDHC");
  } 
  else {
     Serial.println("UNKNOWN");
  }

}

void loop() {

  // Check if data is ready.
  if ( kxAccel.dataReady() )
  {
    kxAccel.getAccelData(&myData);
    if ( set == false) {
      setFunc(); // calculates the zerog offset
    }
  } 
  else {
    //resetFunc();
    Serial.print("Data not ready");
  }

  //constants calculations
  accel = (myData.xData * gravity) - zO;
  vel =  /*lastVel +*/ ((accel /*- lastAccel)*/ * 0.033)); //Possibly needs reworking - a*t
                       //curVel = curVel + vel;//current velocity update
                        // detection from idle
  if (accel >= 0.5 && detM == false) { // absolute value to account for positive and negative change
    curVel = curVel + vel;//current velocity update
    Serial.print("V:");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Detected "); //Debug purposes
    String startTime = RTC(); //timestamp
    Serial.println();
    File file = SD.open("/accelerationdata.txt", FILE_APPEND); //open file.txt to write data
    if(!file) {
    Serial.println("Could not open file(writing).");
    }
    else {
      file.print("Start Time of Motion Detected At: ");
      file.print(startTime);
      file.println();
      file.close();
    }
    detM = true;
    calc = true;
  } 
  else if ((curVel) < 0.0 && detM == true) { // stop detection, parameters need testing
    Serial.print(" V: ");
    Serial.print(curVel, 4);
    Serial.println();
    Serial.print(" A: ");
    Serial.print(accel, 4);
    Serial.println();
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Stopped ");
    String stopTime = RTC();
    Serial.println();
    File file = SD.open("/accelerationdata.txt", FILE_APPEND); //open file.txt to write data
    if(!file) {
    Serial.println("Could not open file(writing).");
    }
    else {
      file.print("Stop Time of Motion Detected At: ");
      file.print(stopTime);
      file.println();
      file.close();
    }
    detM = false;
    calc = false;
    curVel = 0.0;
  } 
  else {
    if(calc == true){
      curVel =curVel + vel;      
    }
    Serial.print(" cV: ");
    Serial.print(curVel, 4);
    Serial.print(" V: ");
    Serial.print(vel, 4);
    Serial.println();
    Serial.print("L A: ");
     Serial.print((accel), 4);
    Serial.print(" lA: ");
    //Serial.print((lastAccel), 4);
    Serial.println();
  }
  //sets last values
  lastVel = vel;
  lastAccel = accel;

  delay(33.33); // (Delay should be 1/ODR (Output Data Rate), default was 1/50ODR(adjusted)50hz) now is 10Hz

} //EOF
