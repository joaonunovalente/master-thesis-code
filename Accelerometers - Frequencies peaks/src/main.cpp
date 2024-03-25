/*
========================================================================
              Frequency Analysis with Signal Generation and FFT
========================================================================

This code performs frequency analysis utilizing signal generation and Fast Fourier Transform (FFT).

Functionalities and Tasks:

- Generates multiple sine wave signals with varying frequencies.
- Combines generated signals to create a composite signal for analysis.
- Executes Fast Fourier Transform (FFT) on the composite signal.
- Identifies peaks in the frequency domain of the signal.

Signal Generation Details:
- Generates multiple sine wave signals with frequencies: 25Hz, 40Hz, 90Hz, 110Hz, and 155Hz.
- Signal amplitude is set to 127.

FFT Configuration:
- Number of samples for signal generation and FFT: 512
- Sampling frequency: 500Hz

Author:   João Nuno Valente
Email:    jnvalente@ua.pt
Date:     September, 2023

========================================================================
*/
#include <Arduino.h>
#include <arduinoFFT.h>
#include <vector>
#include <math.h>

// Define constants for signal generation and FFT
const int numSamples = 512;   // Number of samples for each signal
const float sampleRate = 500; // Sampling rate in Hz
const float f1 = 25.0;        // Frequency for signal 1
const float f2 = 40.0;        // Frequency for signal 2
const float f3 = 90.0;        // Frequency for signal 3
const float f4 = 110;         // Frequency for signal 4
const float f5 = 155;         // Frequency for signal 5
const int amplitude = 127;    // Amplitude for sine waves

// Arrays to hold individual signals
std::vector<double> F1(numSamples);
std::vector<double> F2(numSamples);
std::vector<double> F3(numSamples);
std::vector<double> F4(numSamples);
std::vector<double> F5(numSamples);

// Define parameters and arrays for FFT analysis
#define SAMPLES 512       // Total samples for FFT
#define SAMPLING_FREQ 500 // Sampling frequency for FFT in Hz
arduinoFFT FFT = arduinoFFT();
std::vector<double> frequency(SAMPLES / 2); // Frequencies after FFT
std::vector<double> magnitude(SAMPLES);     // Magnitudes after FFT
std::vector<double> vReal(SAMPLES);         // Real part of the FFT
std::vector<double> vImag(SAMPLES);         // Imaginary part of the FFT
std::vector<double> accelerations(SAMPLES); // Composite signal summing individual signals

// Function to identify peaks in FFT magnitude data
std::vector<std::pair<double, int>> findPeaks(const std::vector<double> &data, const std::vector<double> &frequencies)
{
  // Calculate the sum of magnitudes
  double sum = 0.0;
  for (double element : data)
  {
    sum += element;
  }

  // Calculate the average magnitude
  double average = sum / data.size();
  Serial.println("Average Magnitude: " + String(average));

  std::vector<std::pair<double, int>> peaks; // Store frequency and index of peaks

  // Find peaks by comparing magnitudes with the average
  for (int i = 1; i < data.size() - 1; i++)
  {
    if (data[i] >= data[i - 1] && data[i] >= data[i + 1] && data[i] >= average)
    {
      // The current element is a peak
      double interpolatedFrequency = frequencies[i - 1] + (frequencies[i] - frequencies[i - 1]) / 2;
      peaks.push_back(std::make_pair(interpolatedFrequency, i)); // Store peak frequency and index
    }
  }

  return peaks;
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Generate individual sine wave signals and calculate composite signal
  for (int i = 0; i < numSamples; i++)
  {
    float t = i / sampleRate; // Time period calculation
    F1[i] = amplitude * sin(2 * PI * f1 * t);
    F2[i] = amplitude * sin(2 * PI * f2 * t);
    F3[i] = amplitude * sin(2 * PI * f3 * t);
    F4[i] = amplitude * sin(2 * PI * f4 * t);
    F5[i] = amplitude * sin(2 * PI * f5 * t);
    accelerations[i] = F1[i] + F2[i] + F3[i] + F4[i] + F5[i]; // Composite signal from individual signals
  }

  // Calculate frequencies for FFT
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    frequency[i] = (i * 1.0 * SAMPLING_FREQ) / SAMPLES;
  }

  // Perform FFT on composite signal
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

  // Select the first half of the magnitudes for analysis
  std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);

  // Identify peaks in the frequency domain
  std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);

  // Print half of the magnitudes obtained from FFT
  for (int i = 0; i < SAMPLES / 2; i++)
  {
    Serial.println(magnitudeHalf[i]);
  }

  // Output all the peak values and their indices obtained from findPeaks function
  for (const auto &peak : peakInfo)
  {
    double peakValue = peak.first;
    int peakIndex = peak.second;
    Serial.print("Peak index: " + String(peakIndex) + " | ");
    Serial.println("Frequency: " + String(peakValue));
  }
}

void loop()
{
  // Empty loop as all operations are performed in setup()
}
