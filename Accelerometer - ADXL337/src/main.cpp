/*
========================================================================
              Accelerometer Data Collection with Analog Sensors
========================================================================

This code reads analog sensor values from three axes (X, Y, Z) and
converts them into acceleration values in meters per second squared (m/s^2).

Functionalities and Tasks:

- Reads analog sensor values from X, Y, and Z-axis pins.
- Calculates acceleration from the sensor readings.
- Applies calibration and sensitivity adjustments for accurate data.
- Prints the calculated accelerations in m/s^2 through serial communication.

Sensor Configuration:
- Sensitivity for X and Y axes: 0.300
- Sensitivity for Z axis: 0.300

Author:   João Nuno Valente
Email:    jnvalente@ua.pt
Date:     September, 2023

========================================================================
*/

#include <Arduino.h>

const int xPin = 34;                // Pin connected to the X-axis sensor
const int yPin = 39;                // Pin connected to the Y-axis sensor
const int zPin = 36;                // Pin connected to the Z-axis sensor
const float sensitivity_xy = 0.300; // Sensitivity for X and Y axes
const float sensitivity_z = 0.300;  // Sensitivity for Z axis

void setup()
{
  pinMode(xPin, INPUT); // Set X-axis pin as input
  pinMode(yPin, INPUT); // Set Y-axis pin as input
  pinMode(zPin, INPUT); // Set Z-axis pin as input

  Serial.begin(115200); // Initialize serial communication
  delay(1000);          // Delay for stability after serial initialization
}

void loop()
{
  int analogValue_x = analogRead(xPin); // Read analog value from X-axis pin
  int analogValue_y = analogRead(yPin); // Read analog value from Y-axis pin
  int analogValue_z = analogRead(zPin); // Read analog value from Z-axis pin

  // Calculate the difference between midpoint and sensor readings
  float difference_x = 4096 / 2 - analogValue_x;
  float difference_y = 4096 / 2 - analogValue_y;
  float difference_z = 4096 / 2 - analogValue_z;

  // Convert analog readings to voltage
  float voltage_x = analogValue_x * (3.3 / 4095);
  float voltage_y = analogValue_y * (3.3 / 4095);
  float voltage_z = analogValue_z * (3.3 / 4095);

  // Center the values of the acceleration around 0
  float deltaV_x = voltage_x - (3.3 / 2.0);
  float deltaV_y = voltage_y - (3.3 / 2.0);
  float deltaV_z = voltage_z - (3.3 / 2.0);

  // Convert voltage to acceleration
  float acceleration_x = deltaV_x / sensitivity_xy;
  float acceleration_y = deltaV_y / sensitivity_xy;
  float acceleration_z = deltaV_z / sensitivity_z;

  // Calibrate the values to account for sensor offset
  acceleration_x = acceleration_x + 0.43;
  acceleration_y = acceleration_y + 0.57;
  acceleration_z = acceleration_z + 0.4;

  // Convert acceleration to m/s^2
  float acceleration_x_ms2 = acceleration_x * 9.81; // Convert X-axis acceleration to m/s^2
  float acceleration_y_ms2 = acceleration_y * 9.81; // Convert Y-axis acceleration to m/s^2
  float acceleration_z_ms2 = acceleration_z * 9.81; // Convert Z-axis acceleration to m/s^2

  // Print the calculated accelerations in m/s^2
  Serial.print(acceleration_x_ms2);
  Serial.print(" || ");
  Serial.print(acceleration_y_ms2);
  Serial.print(" || ");
  Serial.println(acceleration_z_ms2);

  delay(100); // Delay to control the output rate and stability
}