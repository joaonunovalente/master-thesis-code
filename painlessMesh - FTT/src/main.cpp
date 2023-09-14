//************************************************************
//
//
//
//************************************************************
#include <Arduino.h>
// Libraries for painlessMesh
#include "painlessMesh.h"
#include "WiFi.h"
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
// ------> painlessMesh
#define MESH_PREFIX "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// ------> Accelerometer
Ticker timer;         // Ticker object to control the measurement interval
Adafruit_MPU6050 mpu; // Create an instance of the MPU6050 class

const int numSamples = 256;                // Number of samples to collect
float accelerometerSamples[numSamples][3]; // Array to store accelerometer samples
int sampleCount = 0;                       // Counter for the current sample
int loopCounter = 1;                       // Counter for the current loop

// -------> FFT
#define SAMPLES 256       // Number of samples
#define SAMPLING_FREQ 500 // Sampling frequency in Hz

arduinoFFT FFT = arduinoFFT();

std::vector<double> frequency(SAMPLES / 2);
std::vector<double> magnitude(SAMPLES);
std::vector<double> vReal(SAMPLES);
std::vector<double> vImag(SAMPLES);
std::vector<double> accelerations(SAMPLES);
std::vector<double> peakValues;
// <----------------->

double peakValueArray[5];
double indexValueArray[5];

// Auxiliar Function
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
    if (data[i] >= data[i - 1] && data[i] >= data[i + 1] && data[i] >= average * 10)
    {
      // The current element is a peak
      double interpolatedFrequency = frequencies[i - 1] + (frequencies[i] - frequencies[i - 1]) / 2;
      peaks.push_back(std::make_pair(interpolatedFrequency, i));
    }
  }
  return peaks;
}

// Prototype so PlatformIO doesn't complain
void sendAccelerometerSamples();
void sampleAccelerometer();
void computeFFT();
void measureAccelerations();
Task taskMeasureAccelerations(TASK_SECOND * 10, TASK_FOREVER, &measureAccelerations);

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("%u | %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // Initialize the MPU6050
  mpu.begin();

  // <--Accelerometer-->
  // Configure the accelerometer range, gyro range, and filter bandwidth
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  // <----------------->

  userScheduler.addTask(taskMeasureAccelerations);
  taskMeasureAccelerations.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}

void measureAccelerations()
{
  // Attach the timer interrupt to call the sampleAccelerometer function every 1 millisecond (1ms)
  sampleCount = 0;
  timer.attach_ms(1, sampleAccelerometer);
}

void sampleAccelerometer()
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
  accelerometerSamples[sampleCount][0] = a.acceleration.x;
  accelerometerSamples[sampleCount][1] = a.acceleration.y;
  accelerometerSamples[sampleCount][2] = a.acceleration.z;

  sampleCount++;

  // Check if the desired number of samples is reached
  if (sampleCount >= SAMPLES)
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
    Serial.print("Sample: " + String(i + 1) + " | ");
    Serial.print(accelerometerSamples[i][2]);
    Serial.print(" | ");
    Serial.print(accelerometerSamples[i + SAMPLES / 2][2]);
    Serial.println(" | " + String(magnitude[i]));
  }
  std::vector<double> magnitudeHalf = std::vector<double>(magnitude.begin(), magnitude.begin() + SAMPLES / 2);
  std::vector<std::pair<double, int>> peakInfo = findPeaks(magnitudeHalf, frequency);
  Serial.println("------------ THE PEAKS -----------");
  peakValues.clear();
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
      peakValues.push_back(peakValue);
      int peakIndex = peak.second;
    }
  }
  Serial.println("------------ The values -----------");
  sendAccelerometerSamples();
}

void sendAccelerometerSamples()
{
  String msg = "";
  // Iterate through the collected samples
  for (int i = 0; i < SAMPLES; i++)
  {
    msg += accelerometerSamples[i][0];
    msg += " | ";
    msg += accelerometerSamples[i][1];
    msg += " | ";
    msg += accelerometerSamples[i][2];
    msg += " | ";
  }
  msg += "| Peak: ";
  for (int i = 0; i < peakValues.size(); i++)
  {
    msg += peakValues[i];
    msg += " ; ";
  }

  msg += "||";
  // Broadcast the accelerometer samples
  mesh.sendBroadcast(msg);
  std::vector<double> peakValues;
}