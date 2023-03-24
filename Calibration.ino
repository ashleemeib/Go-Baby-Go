// These constants won't change:
const int accelerometerPin = EVI 13;  // pin that the accelerometer is attached to


// variables:
float accelerometerValue = 0;   // the sensor value
float accelerometerMin = 1023;  // minimum accelerometer value, change the float min value
float accelerometerMax = 0;     // maximum accelerometer value


void setup() {
  // start of the calibration period:

  // calibrate during the first five seconds
  while (millis() < 5000) {
    accelerometerValue = analogRead(accelerometerPin);

    // record the maximum accelerometer value
    if (accelerometerValue > accelerometerMax) {
      accelerometerMax = accelerometerValue;
    }

    // record the minimum accelerometer value
    if (accelerometerValue < accelerometerMin) {
      accelerometerMin = accelerometerValue;
    }
  }

}

void loop() {
  // read the accelerometer:
  accelerometerValue = analogRead(accelerometerPin);

  // in case the sensor value is outside the range seen during calibration
  accelerometerValue = constrain(accelerometerValue, accelerometerMin, accelerometerMax);

   // apply the calibration to the accelerometer reading
  accelerometerValue = map(accelerometerValue, accelerometerMin, accelerometerMax, 0, 255);


}
