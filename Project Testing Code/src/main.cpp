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
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ticker.h>
#include <arduinoFFT.h>
#include <vector>
#include <math.h>

Ticker timer;         // Create a timer object for periodic measurements
Adafruit_MPU6050 mpu; // Create an MPU6050 accelerometer/gyroscope object

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
double magnitudeArray[SAMPLES][3];

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
    Serial.begin(115200);                         // Initialize serial communication
    mpu.begin();                                  // Initialize the MPU6050 sensor
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // Set accelerometer range to +/- 4g
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);      // Set gyroscope range to +/- 500 degrees per second
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);    // Set filter bandwidth to 5 Hz
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

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp); // Read accelerometer data from MPU6050

    // Store accelerometer data with adjustments in the accelerometerSamples array
    accelerometerSamples[sampleCount][0] = a.acceleration.x - 0.29;
    accelerometerSamples[sampleCount][1] = a.acceleration.y + 0.36;
    accelerometerSamples[sampleCount][2] = a.acceleration.z + 1.07 - 0.52;

    sampleCount++; // Increment sample count

    if (sampleCount >= SAMPLES)
    {
        timer.detach(); // Stop the timer
        computeFFT();   // Perform FFT on collected data
    }
}

void computeFFT()
{
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < SAMPLES; i++)
        {
            vReal[i] = 0;
            vImag[i] = 0;
            magnitude[i] = 0;
        }

        // Prepare data for FFT
        for (int i = 0; i < SAMPLES; i++)
        {
            accelerations[i] = accelerometerSamples[i][j]; // Use z-axis accelerometer data
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

        // Find peaks in the FFT magnitude spectrum
        std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);
        std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);

        if (peakInfo.empty())
        {
        }
        else
        {
            for (const auto &peak : peakInfo)
            {
                double peakValue = peak.first;
                int peakIndex = peak.second;
            }
        }

        FFT.ComplexToMagnitude(magnitude.data(), vImag.data(), SAMPLES);
        for (int i = 0; i < SAMPLES; i++)
        {
            magnitudeArray[i][j] = magnitude[i];
        }
    }

    // Print FFT results
    for (int i = 0; i < SAMPLES / 2; i++)
    {
        Serial.print("Sample: ");
        Serial.print(i + 1);
        Serial.print(" || " + String(magnitudeArray[i][0]));
        Serial.print(" | " + String(magnitudeArray[i][1]));
        Serial.println(" | " + String(magnitudeArray[i][2]));
    }
    Serial.println("------------ THE PEAKS -----------");
    for (int j = 0; j < 3; j++)
    {
    }

    Serial.println("------------ The values -----------");
}
