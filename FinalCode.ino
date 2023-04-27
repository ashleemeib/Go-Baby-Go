#include <Arduino.h>
#line 1 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
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
#include <SPI.h>
#include <Wire.h>
#include <SparkFun_RV8803.h>
#include "SparkFun_KX13X.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"


//variables
RV8803 rtc;
SparkFun_KX134 kxAccel;
outputData myData;   // Struct for the accelerometer's data
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
const float gravity = 9.80665;  // earth's gravity in m/s^2

float accel;
int timet = 100; // samples for acceleration data
bool set = false;
bool det = false;
float last_x;
float last_y;
float last_z;
float x;
float y;
float z;
float move_tholdX = 0.1;
float move_tholdY = 0.1;
float move_tholdZ = 0.1;
int stopped = 0;


//Pins
#define EVI 13 // timestamp pin

//timestamp subroutine
#line 50 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
String RTC();
#line 95 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
void accelRead();
#line 127 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
void sd_Start();
#line 143 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
void sd_Stop();
#line 159 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
void setup();
#line 237 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
void loop();
#line 50 "C:\\Users\\18083\\AppData\\Local\\Temp\\arduino_modified_sketch_884958\\acceldemo.ino"
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
//void(* resetFunc) (void) = 0; //declare reset function @ address 0

//initialize
/*void setFunc() {
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
  }*/
void accelRead() {

  // new sensor event
  sensors_event_t event;

  float x_sum = 0;
  float y_sum = 0;
  float z_sum = 0;

  // Take average of 50 samples for each axis
  for (int i = 0; i < 50; i++) {
    lis.getEvent(&event);
    x_sum += event.acceleration.x;
    y_sum += event.acceleration.y;
    z_sum += event.acceleration.z;
    delay(2);
  }

  last_x = x;
  last_y = y;
  last_z = z;

  x = x_sum / 50.0;
  y = y_sum / 50.0;
  z = z_sum / 50.0;

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print(" \tX: "); Serial.print(x);
  Serial.print(" \tY: "); Serial.print(y);
  Serial.print(" \tZ: "); Serial.print(z);
  Serial.println(" m/s^2 ");
}
void sd_Start() {

  String startTime = RTC(); //timestamp
  Serial.println();
  File file = SD.open("/accelerationdata.txt", FILE_APPEND); //open file.txt to write data
  if (!file) {
    Serial.println("Could not open file(writing).");
  }
  else {
    file.print("Start Time of Motion Detected At: ");
    file.print(startTime);
    file.println();
    file.close();
  }

}
void sd_Stop() {

  String stopTime = RTC(); //timestamp
  Serial.println();
  File file = SD.open("/accelerationdata.txt", FILE_APPEND); //open file.txt to write data
  if (!file) {
    Serial.println("Could not open file(writing).");
  }
  else {
    file.print("Time of Stop Detected At: ");
    file.print(stopTime);
    file.println();
    file.close();
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
  /*if ( !kxAccel.begin() )
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
  */
  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");

  // Set up lis and init readings
  lis.setRange(LIS3DH_RANGE_4_G);
  delay(10);
  lis.read();
  //orientation adjustment
  x = -lis.z;
  y = lis.y;
  z = -lis.x;

  // Check if SD is connected correctly
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  }
  else {
    Serial.println("UNKNOWN");
  }

  x = 0.0;
  y = 0.0;
  z = 0.0;
}

void loop() {
  // Accelerometer
  // Uses a delta scheme to detect movement
  accelRead();
  //Serial.print(abs(last_x - x));
  //Serial.print(" ");
  //Serial.print(abs(last_y - y));
  //Serial.print(" ");
  //Serial.print(abs(last_z - z));
  //Serial.println();
  // If there is a significant delta, time stamp and record data to SD card
  /*if ( abs(last_x - x) > move_tholdX &&
       abs(last_y - y) > move_tholdY &&
       abs(last_z - z) > move_tholdZ && det == false) {
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    //Serial.print("Detected "); //Debug purposes
    det = true;
    sd_Start();
  } else if (move_tholdX > abs(last_x - x) &&
             move_tholdY > abs(last_y - y) &&
             move_tholdY > abs(last_z - z)
             && det == true
             && stopped < 3) {
    stopped += 1;
  } else if (move_tholdX > abs(last_x - x) &&
             move_tholdY > abs(last_y - y) &&
             move_tholdY > abs(last_z - z)
             && det == true
             && stopped == 3) {//stop recorded
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    //Serial.print("Stopped "); //Debug purposes
    sd_Stop();
    det = false;
    stopped = 0;
  } else {
    stopped = 0;
  }
*/
  // Give time between readings
  delay(100);

  /*
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
    //accel = (myData.xData * gravity) - zO;
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
  */

} //EOF
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
