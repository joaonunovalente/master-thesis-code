#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ticker.h>

Ticker timer;         // Ticker object to control the measurement interval
Adafruit_MPU6050 mpu; // Create an instance of the MPU6050 class

const int samplyingFrequency = 500;
const int numSamples = 600;                // Number of samples to collect
float accelerometerSamples[numSamples][3]; // Array to store accelerometer samples
int sampleCount = 0;                       // Counter for the current sample
int loopCounter = 1;                       // Counter for the current loop

void printAccelerometerSamples()
{
  // Iterate through the collected samples
  for (int i = 0; i < numSamples; i++)
  {
    // Print the sample number and its X, Y, Z values
    Serial.print("Sample ");
    Serial.print(i + 1);
    Serial.print(": X = ");
    Serial.print(accelerometerSamples[i][0]);
    Serial.print(" | Y = ");
    Serial.print(accelerometerSamples[i][1]);
    Serial.print(" | Z = ");
    Serial.println(accelerometerSamples[i][2]);
  }
}

void sampleAccelerometer()
{
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Store the values in the array */
  accelerometerSamples[sampleCount][0] = a.acceleration.x - 0.29;
  accelerometerSamples[sampleCount][1] = a.acceleration.y + 0.36;
  accelerometerSamples[sampleCount][2] = a.acceleration.z + 1.07 - 0.52;

  sampleCount++;

  // Check if the desired number of samples is reached
  if (sampleCount >= numSamples)
  {
    // Stop the timer interrupt
    timer.detach();

    // Call the function to print the accelerometer samples
    printAccelerometerSamples();
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize the MPU6050
  mpu.begin();

  // Configure the accelerometer range, gyro range, and filter bandwidth
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void loop()
{
  // Print loop number
  Serial.print("Loop: ");
  Serial.println(loopCounter);
  loopCounter++;

  sampleCount = 0;
  // In miliseconds
  timer.attach_ms(1 / samplyingFrequency, sampleAccelerometer);

  delay(2000);
}
