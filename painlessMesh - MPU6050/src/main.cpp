//************************************************************
//
//
//
//************************************************************
#include "painlessMesh.h"
#include "WiFi.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ticker.h>

#define MESH_PREFIX "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// // <--Accelerometer-->
Ticker timer;         // Ticker object to control the measurement interval
Adafruit_MPU6050 mpu; // Create an instance of the MPU6050 class

const int numSamples = 500;               // Number of samples to collect
float accelerometerSamples[numSamples][3]; // Array to store accelerometer samples
int sampleCount = 0;                       // Counter for the current sample
int loopCounter = 1;                       // Counter for the current loop
// <-------------------->
void sendAccelerometerSamples()
{
  String msg = "";
  // Iterate through the collected samples
  for (int i = 0; i < numSamples; i++)
  {

    // // Prepare the string with the sample number and its X, Y, Z values
    // msg += "Sample ";
    // msg += i + 1;
    // msg += " | X = ";
    // msg += accelerometerSamples[i][0];
    // msg += " | Y = ";
    // msg += accelerometerSamples[i][1];
    // msg += " | Z = ";
    // msg += accelerometerSamples[i][2];
    // msg += " || ";

    // Prepare the string with the sample number and its X, Y, Z values

    msg += accelerometerSamples[i][0];
    msg += "|";
    msg += accelerometerSamples[i][1];
    msg += "|";
    msg += accelerometerSamples[i][2];
    msg += "||";
  }
  // Broadcast the accelerometer samples
  mesh.sendBroadcast(msg);
}

void sampleAccelerometer()
{
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Store the values in the array */
  accelerometerSamples[sampleCount][0] = a.acceleration.x + 0.20;
  accelerometerSamples[sampleCount][1] = a.acceleration.y - 0.08;
  accelerometerSamples[sampleCount][2] = a.acceleration.z + 0.78;

  sampleCount++;

  // Check if the desired number of samples is reached
  if (sampleCount >= numSamples)
  {
    // Stop the timer interrupt
    timer.detach();

    // Call the function to print the accelerometer samples
    sendAccelerometerSamples();
  }
}

void measureAccelerations(); // Prototype so PlatformIO doesn't complain

Task taskMeasureAccelerations(TASK_SECOND * 10, TASK_FOREVER, &measureAccelerations);

void measureAccelerations()
{
  // Attach the timer interrupt to call the sampleAccelerometer function every 1 millisecond (1ms)
  sampleCount = 0;
  timer.attach_ms(1, sampleAccelerometer);
}

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