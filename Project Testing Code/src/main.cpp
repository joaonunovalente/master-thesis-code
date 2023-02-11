// main.cpp

// *************************************
//                #include
// *************************************
#include <Arduino.h>
#include <WiFi.h>
#include <painlessMesh.h>

// *************************************
//                #Define
// *************************************

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#ifdef LED_BUILTIN
#define LED LED_BUILTIN
#else
#define LED 2
#endif

// Blink #define
#define BLINK_PERIOD 3000  // milliseconds until cycle repeat
#define BLINK_DURATION 100 // milliseconds LED is on for

// Network credentials
#define MESH_SSID "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();

// Objects
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

void setup()
{

  // Intro setup()
  Serial.begin(115200);

  // Mesh inicialization
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  // Blinking
  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []()
                   {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run 
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      } });
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
}

void loop()
{
  mesh.update();
  digitalWrite(LED, !onFlag);

  // Scan nearby Networks
  int numSSID = WiFi.scanNetworks();

  // print the network found:
  for (int thisNet = 0; thisNet < numSSID; thisNet++)
  {
    // List of ESP32 MAC addresses
    String MAC_address[] = {
        "78:E3:6D:18:FE:68",
        "4C:11:AE:CA:92:91",
        "78:E3:6D:0A:23:01",
        "78:21:84:8C:B9:39"};
    // Length of the array MAC_address
    int len_MAC_address = sizeof(MAC_address) / sizeof(MAC_address[0]);

    for (int ii = 0; ii < len_MAC_address; ii++)
    {
      // Prints the MAC_addresses and the RSSI
      // that match the ESP32.
      if (WiFi.BSSIDstr(thisNet) == MAC_address[ii])
      {
        Serial.print("MAC Address: ");
        Serial.print(WiFi.BSSIDstr(thisNet));
        Serial.print(" | ");
        Serial.print(" Signal = ");
        Serial.print(WiFi.RSSI(thisNet));
        Serial.println("dB");
      }
    }
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
}