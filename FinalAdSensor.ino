
//Go Baby Go Adherence Sensor Master Code
//Written by Ashlee B, Kyler M, Tara P 4/26/23
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
int detect = 0;
float timeBlink = 0;
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



//Pins
#define EVI 14 // timestamp pin
#define LED 17 // LED pin

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
//read from the accelerometer function puts data through moving average filter
void accelRead() {

  // new sensor event
  sensors_event_t event;

  float x_sum = 0;
  float y_sum = 0;
  float z_sum = 0;

  //Moving Average Filter- Take average of 50 samples for each axis
  for (int i = 0; i < 50; i++) {
    lis.getEvent(&event);
    x_sum += (-event.acceleration.z);
    y_sum += event.acceleration.y;
    z_sum += (-event.acceleration.x);
    delay(2);
  }

  last_x = x;
  last_y = y;
  last_z = z;

  x = x_sum / 50.0;
  y = y_sum / 50.0;
  z = z_sum / 50.0;
}
void sd_Start() {//prints to sd card starting

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
void sd_Stop() {//printing to the SD card

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
  pinMode(LED, OUTPUT);
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
  //orientated on the back of the permobil
  x = -lis.z;
  y = lis.y;
  z = -lis.x;

  // Check if SD is connected correctly
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    //   digitalWrite(LED, HIGH);
    // while (1) { }
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    // digitalWrite(LED, HIGH);
    //while (1) { }
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
  //axis zeroing to eliminate false positives
  x = 0;
  y = 0;
  z = 0;
  //
  String startTime = RTC(); //timestamp
  Serial.println();
  File file = SD.open("/accelerationdata.txt", FILE_APPEND); //open file.txt to write data
  if (!file) {
    Serial.println("Could not open file(writing).");
  }
  else {
    file.println();
    file.print("Device start up at: ");
    file.print(startTime);
    file.println();
    file.close();
  }
}

void loop() {
  //reset values before reading, eliminates false positives
  //blink timer
  /* if (timeBlink <= 100) {
     timeBlink += 0.1;
    } else {
     //turn on LED for to indicate device is on
     digitalWrite(LED, HIGH);
     delay(50);
     digitalWrite(LED, LOW);
     timeBlink = 0;
    }*/
  // Accelerometer
  // Uses a delta scheme to detect movement
  accelRead();
  Serial.println(x);

  // If there is a significant delta, time stamp and record data to SD card
  if ( abs(last_x - x) > move_tholdX &&
       abs(last_y - y) > move_tholdY &&
       abs(last_z - z) > move_tholdZ && det == false) {
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Detected "); //Debug purposes
    det = true;
    sd_Start();
  } else if (move_tholdX > abs(last_x - x) &&
             move_tholdY > abs(last_y - y) &&
             move_tholdY > abs(last_z - z) && detect < 3 && det == true) {
    detect += 1;

  } else if (move_tholdX > abs(last_x - x) &&
             move_tholdY > abs(last_y - y) &&
             move_tholdY > abs(last_z - z) && detect == 3 && det == true) {
    digitalWrite(EVI, HIGH);   // trigger EVI pin
    delay(20);                       // wait for a second
    digitalWrite(EVI, LOW);    // turn off EVI pin output
    Serial.print("Stopped "); //Debug purposes
    sd_Stop();
    det = false;
    detect = 0;
  } else {//if the data goes above the thresholds over reset detect counter
    detect = 0;
  }
  // Give time between readings
  delay(100);

} //EOF
