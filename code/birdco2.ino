#include "SparkFun_SCD4x_Arduino_Library.h"
#include "motor.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <cmath>

// Define the compile-time flag
#define USE_SERIAL_LOGGING // Comment this line to disable serial logging
#define HAS_CO2_SENSOR     // Comment this line out to disable CO2 sensor

// Wires from Node pins definition
//
// Servo    --> ESP8266
//   Brown  --> GND (Ground)
//   Red    --> 5V
//   Yellow --> D3  --> here GPIO 0 in code
//
// SDC41    --> ESP8266
// (CO2 sensor)
//   SDA    --> D1  --> GPIO 4 in code
//   SCL    --> D2  --> GPIO 5 in code
//   VCC    --> 5V
//   GND    --> GND

const unsigned long MEASUREMENT_INTERVAL = 5 * 60 * 1000;
const unsigned long LOOP_DELAY = 10 * 1000;

const int SERVO_PIN = 0;
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;
const int CO2_MIN_THRESHOLD = 600;  // in ppm --> Bird will be upright
const int CO2_MAX_THRESHOLD = 1000; //       --> Bird will be fully hanging down
const int SERVO_STEP_DELAY = 20;   //
const int MIN_DIFFERENCE = 10;      // ppm difference needed for bird to move

const int SDA_PIN = 4; // SCD41 orange wire
const int SCL_PIN = 5; // SCD41 yellow wire

unsigned long last_co2_time = 0;
int current_co2_value = CO2_MIN_THRESHOLD;
int last_co2_value = CO2_MIN_THRESHOLD;
int last_servo_angle = (ANGLE_MIN + ANGLE_MAX) / 2; // Default position

Motor bird_servo(SERVO_PIN);
#ifdef HAS_CO2_SENSOR
SCD4x co2_sensor(SCD4x_SENSOR_SCD41);
#endif

void setup() {
#ifdef USE_SERIAL_LOGGING
  Serial.begin(115200);
  Serial.println("Starting up");
#endif

  // We dont need wifi and as such we turn it off
  WiFi.mode(WIFI_OFF);
#ifdef USE_SERIAL_LOGGING
  Serial.println("Disabled WIFI");
#endif

  bird_servo.attach();
#ifdef USE_SERIAL_LOGGING
  Serial.println("Attached to servo motor");
#endif

#ifdef HAS_CO2_SENSOR
  Wire.begin(SDA_PIN, SCL_PIN);
#ifdef USE_SERIAL_LOGGING
  Serial.println("Enabled communication with CO2 sensor");
#endif
#endif

#ifdef HAS_CO2_SENSOR
#ifdef USE_SERIAL_LOGGING
  Serial.println("Trying to read from sensor");
#endif
  // This configures the sensors settings
  if (!co2_sensor.begin(false, true, false)) {
#ifdef USE_SERIAL_LOGGING
    Serial.println("Failed to initialize sensor!");
#endif
    while (1)
      ;
  }
  //
  if (!co2_sensor.measureSingleShot()) {
#ifdef USE_SERIAL_LOGGING
    Serial.println("Failed to take a measurement!");
#endif
    while (1)
      ;
  }

#endif
#ifdef USE_SERIAL_LOGGING
  Serial.println("Setup complete");
  // test_setup();
#endif
}

void test_setup() {
#ifdef USE_SERIAL_LOGGING
  Serial.println("Testing setup");
#endif

  // test angles
  int angles[] = {0, 90, 180};
  for (int i = 0; i < 2; i++) {
    Serial.print("Motor to: ");
    Serial.print(angles[i]);
    bird_servo.setPosition(angles[i], SERVO_STEP_DELAY);
    delay(300);
  }
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - last_co2_time >= MEASUREMENT_INTERVAL) {
#ifdef USE_SERIAL_LOGGING
    Serial.println("Measuring CO2 and adjusting position");
#endif
    last_co2_time = currentTime;

    unsigned long startTime = millis();

#ifdef HAS_CO2_SENSOR
    while (co2_sensor.readMeasurement() == false) {
      Serial.print(F("."));
      delay(500);
    }
    if (co2_sensor.measureSingleShot()) {
      current_co2_value = co2_sensor.getCO2();
#ifdef USE_SERIAL_LOGGING
      Serial.print("Got CO2 measurment of: ");
      Serial.println(current_co2_value);

#endif

    } else {
#ifdef USE_SERIAL_LOGGING
      Serial.println("SingleShot failed");
#endif

      return;
    }
#endif
    if (std::abs(current_co2_value - last_co2_value) >= MIN_DIFFERENCE) {

      int current_servo_angle =
          (current_co2_value < CO2_MIN_THRESHOLD) ? ANGLE_MIN
          : (current_co2_value > CO2_MAX_THRESHOLD)
              ? ANGLE_MAX
              : map(current_co2_value, CO2_MIN_THRESHOLD, CO2_MAX_THRESHOLD,
                    ANGLE_MIN, ANGLE_MAX);

      current_servo_angle =
          constrain(current_servo_angle, ANGLE_MIN, ANGLE_MAX);

      if (current_servo_angle != last_servo_angle) {
        bird_servo.setPosition(current_servo_angle, SERVO_STEP_DELAY);

        last_servo_angle = current_servo_angle;
      }
      last_co2_value = current_co2_value;
#ifdef USE_SERIAL_LOGGING
      Serial.print("CO2 Value: ");
      Serial.print(current_co2_value);
      Serial.print("\nNew Servo Position: ");
      Serial.println(current_servo_angle);
#endif
    } else {
#ifdef USE_SERIAL_LOGGING
      Serial.println("CO2 Value not different enough to move yet.");
      Serial.print("It was: ");
      Serial.println(last_co2_value);
      Serial.print("Now is: ");
      Serial.println(current_co2_value);
#endif
    }
  } else {
#ifdef USE_SERIAL_LOGGING
    Serial.println("No new measurement needed yet");
#endif
  }
  delay(LOOP_DELAY);
}
