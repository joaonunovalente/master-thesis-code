#include <Arduino.h>
#include <arduinoFFT.h>
#include <vector>
#include <math.h>

// <----------------->
const int numSamples = 512;
const float sampleRate = 500;
const float f1 = 25.0;
const float f2 = 40.0;
const float f3 = 90.0;
const float f4 = 110;
const float f5 = 155;
const int amplitude = 127;
std::vector<double> F1(numSamples);
std::vector<double> F2(numSamples);
std::vector<double> F3(numSamples);
std::vector<double> F4(numSamples);
std::vector<double> F5(numSamples);
// <----------------->
// <----------------->
#define SAMPLES 512       // Number of samples
#define SAMPLING_FREQ 500 // Sampling frequency in Hz

arduinoFFT FFT = arduinoFFT();

std::vector<double> frequency(SAMPLES / 2);
std::vector<double> magnitude(SAMPLES);
std::vector<double> vReal(SAMPLES);
std::vector<double> vImag(SAMPLES);
std::vector<double> accelerations(SAMPLES);
// <----------------->

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

  for (int i = 0; i < numSamples; i++)
  {
    float t = i / sampleRate; // Time in seconds
    F1[i] = amplitude * sin(2 * PI * f1 * t);
    F2[i] = amplitude * sin(2 * PI * f2 * t);
    F3[i] = amplitude * sin(2 * PI * f3 * t);
    F4[i] = amplitude * sin(2 * PI * f4 * t);
    F5[i] = amplitude * sin(2 * PI * f5 * t);
    accelerations[i] = F1[i] + F2[i] + F3[i] + F4[i] + F5[i];
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
  std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);
  std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);

  for (int i = 0; i < SAMPLES / 2; i++)
  {
    Serial.println(magnitudeHalf[i]);
  }
  // Output all the peak values and their indices
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
  // Generate and print the sine wave values

  // Your main code can go here if needed
}
