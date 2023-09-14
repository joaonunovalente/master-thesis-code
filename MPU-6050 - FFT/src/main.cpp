// Libraries
#include <Arduino.h>
// Libraies for accelerometer
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ticker.h>
// Libraries for FFT
#include <arduinoFFT.h>
#include <vector>
#include <math.h>

// <------ METHODS ------>
// ------> Accelerometer
Ticker timer;         // Ticker object to control the measurement interval
Adafruit_MPU6050 mpu; // Create an instance of the MPU6050 class

const int samplyingFrequency = 500;
const int numSamples = 512;                // Number of samples to collect
float accelerometerSamples[numSamples][3]; // Array to store accelerometer samples
int sampleCount = 0;                       // Counter for the current sample
int loopCounter = 1;                       // Counter for the current loop

// -------> FFT
#define SAMPLES 512       // Number of samples
#define SAMPLING_FREQ 500 // Sampling frequency in Hz

arduinoFFT FFT = arduinoFFT();

std::vector<double> frequency(SAMPLES / 2);
std::vector<double> magnitude(SAMPLES);
std::vector<double> vReal(SAMPLES);
std::vector<double> vImag(SAMPLES);
std::vector<double> accelerations(SAMPLES);
// <----------------->

// Initialization of functions so that PlatformIO doesn't complain
void accelerationsMeasurements();
void computeFFT();
// Funcção Auxiliar
std::vector<std::pair<double, int>> findPeaks(const std::vector<double> &data, const std::vector<double> &frequencies)
{
  double sum = 0.0;
  for (double element : data)
  {
    sum += element;
  }

  // Calculate the average
  double average = sum / data.size();
  Serial.println("Average: " + String(average));

  std::vector<std::pair<double, int>> peaks; // Initialize a vector to store peak frequency and index pairs

  for (int i = 1; i < data.size() - 1; i++)
  {
    if (data[i] >= data[i - 1] && data[i] >= data[i + 1] && data[i] >= average)
    {
      // The current element is a peak
      double interpolatedFrequency = frequencies[i - 1] + (frequencies[i] - frequencies[i - 1]) / 2;
      peaks.push_back(std::make_pair(interpolatedFrequency, i));
    }
  }

  return peaks;
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
  timer.attach_ms(1 / samplyingFrequency, accelerationsMeasurements);

  delay(10000);
}
void accelerationsMeasurements()
{
  // Reset values of vReal, vImag, magnitude
  for (int i = 0; i < SAMPLES; i++)
  {
    vReal[i] = 0;
    vImag[i] = 0; // This is the one who causes problems
    magnitude[i] = 0;
  }
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

    // Call the function to compute the FFT
    computeFFT();
  }
}
void computeFFT()
{
  for (int i = 0; i < SAMPLES; i++)
  {
    accelerations[i] = accelerometerSamples[i][2];
  }
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    frequency[i] = (i * 1.0 * SAMPLING_FREQ) / SAMPLES;
  }

  // Perform FFT
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
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    Serial.print("Freq: " + String(frequency[i]) + " | ");
    Serial.println("Mag: " + String(magnitude[i]));
  }
  std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);
  std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);
  Serial.println("----------------- THE PEAKS -----------");

  // Shows peaks
  if (peakInfo.empty())
  {
    // Print a message when there are no peaks
    Serial.println("No peaks found.");
  }
  else
  {
    for (const auto &peak : peakInfo)
    {
      double peakValue = peak.first;
      int peakIndex = peak.second;

      // Print the peak index
      Serial.print("Peak index: ");
      Serial.print(peakIndex);

      // Print the peak value
      Serial.print(" | Frequency: ");
      Serial.println(peakValue);
    }
  }
}
