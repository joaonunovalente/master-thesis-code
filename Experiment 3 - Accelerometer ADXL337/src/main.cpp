/*
========================================================================
                  ESP32 Accelerometer Data Analysis Code
========================================================================

This code performs the following tasks:

- Includes necessary libraries for various functionalities.
- Sets up an accelerometer (MPU6050) for data collection.
- Collects accelerometer data samples at a specific sampling frequency.
- Computes the Fast Fourier Transform (FFT) of the collected accelerometer data.
- Identifies peaks in the FFT spectrum, representing significant frequencies.
- Prints loop numbers and accelerometer data during the operation.

Author:   João Nuno Valente
Email:    jnvalente@ua.pt
Date:     September, 2023

========================================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <Ticker.h>
#include <arduinoFFT.h>
#include <vector>
#include <math.h>

Ticker timer; // Create a timer object for periodic measurements

#define SAMPLES 512       // Number of samples for FFT
#define SAMPLING_FREQ 500 // Sampling frequency in Hz

float accelerometerSamples[SAMPLES][3]; // Array to store accelerometer samples
int sampleCount = 0;                    // Counter to keep track of collected samples
int loopCounter = 1;                    // Counter for the main loop

arduinoFFT FFT = arduinoFFT(); // Create an FFT object

std::vector<double> frequency(SAMPLES / 2); // Vector to store frequency values
std::vector<double> magnitude(SAMPLES);     // Vector to store magnitude values
std::vector<double> vReal(SAMPLES);         // Vector to store real part of FFT
std::vector<double> vImag(SAMPLES);         // Vector to store imaginary part of FFT
std::vector<double> accelerations(SAMPLES); // Vector to store accelerations

const int xPin = 34;
const int yPin = 39;
const int zPin = 36;
const float sensitivity_xy = 0.300;
const float sensitivity_z = 0.300;

// Function prototypes
void accelerationsMeasurements();
void computeFFT();

// Function to find peaks in a given data vector and associate them with corresponding frequencies.
std::vector<std::pair<double, int>> findPeaks(const std::vector<double> &data, const std::vector<double> &frequencies)
{
  double sum = 0.0;

  // Calculate the sum of all elements in the data vector
  for (double element : data)
  {
    sum += element;
  }

  // Calculate the average value of the data and multiply by 10
  double average10 = (sum / data.size()) * 10;
  Serial.println("Average10: " + String(average10));

  std::vector<std::pair<double, int>> peaks; // Initialize a vector to store peak frequency and index pairs

  // Iterate through the data vector, excluding the first and last elements
  for (int i = 1; i < data.size() - 1; i++)
  {
    // Check if the current data point is a peak by comparing it to its neighbors and a threshold
    if (data[i] >= data[i - 1] && data[i] >= data[i + 1] && data[i] >= average10)
    {
      // The current element is a peak
      // Calculate an interpolated frequency associated with the peak index
      // double interpolatedFrequency = frequencies[i - 1] + (frequencies[i] - frequencies[i - 1]) / 2;

      // Store the peak frequency and its index in the peaks vector as a pair
      peaks.push_back(std::make_pair(frequencies[i], i));
    }
  }

  // Return a vector containing pairs of peak frequencies and their corresponding indices
  return peaks;
}

void setup()
{
  Serial.begin(115200); // Initialize serial communication
}

void loop()
{
  Serial.print("Loop: ");
  Serial.println(loopCounter);
  loopCounter++;

  sampleCount = 0;                                               // Reset sample count
  timer.attach_ms(1 / SAMPLING_FREQ, accelerationsMeasurements); // Set up timer to collect accelerometer data
  delay(5000);                                                   // Delay for 5 seconds between measurements
}

void accelerationsMeasurements()
{
  for (int i = 0; i < SAMPLES; i++)
  {
    vReal[i] = 0;
    vImag[i] = 0;
    magnitude[i] = 0;
  }

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

  accelerometerSamples[sampleCount][0] = acceleration_x_ms2;
  accelerometerSamples[sampleCount][1] = acceleration_y_ms2;
  accelerometerSamples[sampleCount][2] = acceleration_z_ms2;

  sampleCount++; // Increment sample count

  if (sampleCount >= SAMPLES)
  {
    timer.detach(); // Stop the timer
    computeFFT();   // Perform FFT on collected data
  }
}

void computeFFT()
{
  // Prepare data for FFT
  for (int i = 0; i < SAMPLES; i++)
  {
    accelerations[i] = accelerometerSamples[i][2]; // Use z-axis accelerometer data
  }

  // Calculate frequencies for FFT
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    frequency[i] = (i * 1.0 * SAMPLING_FREQ) / SAMPLES;
  }

  FFT.Windowing(accelerations.data(), SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);

  for (int i = 0; i < SAMPLES; i++)
  {
    vReal[i] = accelerations[i];
  }

  FFT.Compute(vReal.data(), vImag.data(), SAMPLES, FFT_FORWARD);

  for (int i = 0; i < SAMPLES; i++)
  {
    magnitude[i] = vReal[i];
  }

  FFT.ComplexToMagnitude(magnitude.data(), vImag.data(), SAMPLES);

  // Print FFT results
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    Serial.print("Sample: ");
    Serial.print(i + 1);
    Serial.print(" | ");
    Serial.print(accelerometerSamples[i][2]);
    Serial.print(" | ");
    Serial.print(accelerometerSamples[i + SAMPLES / 2][2]);
    Serial.println(" | " + String(magnitude[i]));
  }

  // Find peaks in the FFT magnitude spectrum
  std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);
  std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);
  Serial.println("------------ THE PEAKS -----------");

  if (peakInfo.empty())
  {
    Serial.println("No peaks found.");
  }
  else
  {
    for (const auto &peak : peakInfo)
    {
      double peakValue = peak.first;
      int peakIndex = peak.second;

      Serial.print("Peak index: ");
      Serial.print(peakIndex);
      Serial.print(" | Frequency: ");
      Serial.println(peakValue);
    }
  }
  Serial.println("------------ The values -----------");
}
