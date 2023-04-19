// Libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
#include <SparkFun_RV8803.h>

// Variables
RV8803 rtc;

// Set time between loop cycles
const int ms_between_readings = 100;

// Accelerometer (LIS3DH)
const float move_thold = 0.5;  // Above, time stamp to SD card
const float stop_thold = -0.5; // Below, time stamp to SD card

// Track deltas for each axis
float last_x;   
float last_y;
float last_z;
float x;
float y;
float z;

// Our LIS3DH instance
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup() {
  Serial.begin(115200);
  
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
  x = lis.x;
  y = lis.y;
  z = lis.z;

}

void loop() {
  // Accelerometer
  // Uses a delta scheme to detect movement

  // new sensor event
  sensors_event_t event;

  float x_sum = 0;
  float y_sum = 0;
  float z_sum = 0;

  // Take average of 50 samples for each axis
  for(int i=0; i<50; i++) {
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

  Serial.println();

  // If there is a significant delta, time stamp and record data to SD card
  // if( abs(last_x -x) > move_thold ||
  //     abs(last_y -y) > move_thold ||
  //     abs(last_z -z) > move_thold ){
  //   digitalWrite(acc_led, HIGH);
  // }

  // Give time between readings
  delay(ms_between_readings);
}
