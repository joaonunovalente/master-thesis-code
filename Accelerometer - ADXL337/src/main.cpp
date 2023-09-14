#include <Arduino.h>

const int xPin = 34;
const int yPin = 39;
const int zPin = 36;
const float sensitivity_xy = 0.300;
const float sensitivity_z = 0.300;

void setup()
{
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);

  Serial.begin(115200);
  delay(1000);
}

void loop()
{
  int analogValue_x = analogRead(xPin);
  int analogValue_y = analogRead(yPin);
  int analogValue_z = analogRead(zPin);

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

  // Calibration
  acceleration_x = acceleration_x + 0.43;
  acceleration_y = acceleration_y + 0.57;
  acceleration_z = acceleration_z + 0.4;

  // Convert to m/s^2
  float acceleration_x_ms2 = acceleration_x * 9.81;
  float acceleration_y_ms2 = acceleration_y * 9.81;
  float acceleration_z_ms2 = acceleration_z * 9.81;

  Serial.print(acceleration_x_ms2);
  Serial.print(" || ");
  Serial.print(acceleration_y_ms2);
  Serial.print(" || ");
  Serial.println(acceleration_z_ms2);

  delay(100);
}
