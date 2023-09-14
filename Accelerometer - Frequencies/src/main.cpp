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

#define numPeaks 5
double peaks[numPeaks];
double indices[numPeaks];

int findIndexBetweenValues(const std::vector<double> &vec, double targetValue)
{
  int index = -1; // Default value in case the target value is not found

  for (size_t i = 0; i < vec.size() - 1; ++i)
  {
    if (vec[i] <= targetValue && targetValue <= vec[i + 1])
    {
      index = i;
      break; // Exit the loop once the index is found
    }
  }

  return index;
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
  // FFT.Windowing(accelerations.data(), SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
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

  int index = 0;

  // Delete first n elements

  for (int i = 0; i < numPeaks; i++)
  {

    /* code */
    accelerations.erase(accelerations.begin(), accelerations.begin() + index + 1);
    frequency.erase(frequency.begin(), frequency.begin() + index + 1);
    vReal.erase(vReal.begin(), vReal.begin() + index + 1);
    vImag.erase(vImag.begin(), vImag.begin() + index + 1);
    magnitude.erase(magnitude.begin(), magnitude.begin() + index + 1);

    double peak = FFT.MajorPeak(magnitude.data(), SAMPLES - index, SAMPLING_FREQ - peak);
    if (i == 0)
    {
      peaks[i] = peak;
    }
    else
    {
      peak = peak + peaks[i - 1];
      peaks[i] = peak;
    }
    index = findIndexBetweenValues(frequency, peak);
    indices[i] = index;

    for (int i = 0; i < SAMPLES / 2 - index; i++)
    {
      Serial.print(frequency[i]);
      Serial.print(" Hz | vReal: ");
      Serial.print(vReal[i]);
      Serial.print(" | vImag: ");
      Serial.print(vImag[i]);
      Serial.print(" magnitude: ");
      Serial.println(magnitude[i]);
    }
    Serial.print("Index: ");
    Serial.print(index);
    Serial.print(" & Peak: ");
    Serial.println(peak);
  }

  for (int i = 0; i < numPeaks; i++)
  {
    Serial.print(indices[i]);
    Serial.print(" & ");
    Serial.println(peaks[i]);
  }
}

void loop()
{
  // Generate and print the sine wave values

  // Your main code can go here if needed
}
