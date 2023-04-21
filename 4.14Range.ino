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
//float  zO = 0.0;//zero g offset
float zOx = 0.0; // x direction offset
float zOy = 0.0; // x direction offset
float zOz = 0.0; // x direction offset
int loopVar = 0;
float sample = 0.0;
float lastVel = 0.0;
float vel;
float curVel = 0.0;
float totalx;
float totaly;
float totalz;
float accelx;
float accely;
float accelz;
float lastAccel = 0.0;
float maxX = 0;
float maxY = 0;
float maxZ = 0;
int nR = 0;//not ready count
int startTime = 0;
//int det1 = 0;
int timet = 100;
bool detM = false;//detection movement
bool resetB = false;//reset was used to start the device
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
void readData() {
  loopVar = 5;
  while (loopVar > 0) {
    if ( kxAccel.dataReady() )//
    {
      kxAccel.getAccelData(&myData);
      delay(20);
    }
    totalx = totalx + ((myData.xData * 9.80665) - zOx);
    totaly = totaly + ((myData.yData * 9.80665) - zOy);
    totalz = totalz + ((myData.zData * 9.80665) - zOz);
    loopVar--;
  }
  //averages
  totalx = totalx / 5;
  totaly = totaly / 5;
  totalz = totalz / 5;
  Serial.println(totalx);
  Serial.println(totaly);
  Serial.println(totalz);


}
void setFunc() {//creates zero g offset
  //check if zero g offest records new data
  while (timet > 0) {
    readData();//read data subroutine,
    Serial.println(totalx);
    Serial.println(totaly);
    Serial.println(totalz);
    zOx = zOx + totalx;//x read
    Serial.print("X1: ");
    Serial.print(zOx, 4);//measured in g's then converted to m/s^2
    Serial.println();
    zOy = zOy + totaly;//y read
    Serial.print("Y1: ");
    Serial.print(zOy, 4);//measured in g's then converted to m/s^2
    Serial.println();
    zOz = zOz + totalz;//z read
    Serial.print("Z1: ");
    Serial.print(zOz, 4);//measured in g's then converted to m/s^2
    Serial.println();
    timet --;//decrement
    delay(100);
  }
  Serial.print("SET");
  timet = 100;
  set = true;
  zOx = (zOx) / 100;//zero g axis of x axis
  zOy = (zOy) / 100;//zero g axis of x axis
  zOz = (zOz) / 100;//zero g axis of x axis
  Serial.print(zOx);
  Serial.print(zOy);
  Serial.print(zOz);
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
  kxAccel.setOutputDataRate(50); // Default is 50Hz
  kxAccel.enableAccel();

}

void loop()
{
  totalx = 0.0;//reset the read data
  totaly = 0.0;
  totalz = 0.0;
  // Check if data is ready - redundency for the rest loop
  if ( kxAccel.dataReady() )
  {
    // kxAccel.getAccelData(&myData);
    if (set == false) {
      setFunc();//calculates the zerog offset
    }
  } else if (nR == 20 && resetB == false) {
    resetFunc();
    resetB = true;
  } else {
    Serial.print("data not ready");
    nR += 1;
  }
  readData();
  //constants calculations
  accelx = totalx;
  accely = totaly;
  accelz = totalz;
  /*  if (accelx > maxX) {
      maxX = accelx;
    } else if (accely > maxY) {
      maxY = accely;
    } else if (accelz > maxZ) {
      maxZ = accelz;
    } else {*/
  Serial.println(accelx);
  Serial.println(accely);
  Serial.println(accelz);
  Serial.println(zOx);
  Serial.println(zOy);
  Serial.println(zOz);
  Serial.println(maxX);
  Serial.println(maxY);
  Serial.println(maxZ);
  //}




  delay(100); // (Delay should be 1/ODR (Output Data Rate), default was 1/50ODR(adjusted)50hz)



}//EOF
