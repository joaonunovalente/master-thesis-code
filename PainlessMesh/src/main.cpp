//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 4. prints anything it receives to Serial.print
//
//
//************************************************************
#include <painlessMesh.h>

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#ifdef LED_BUILTIN
#define LED LED_BUILTIN
#else
#define LED 2
#endif

#define BLINK_PERIOD 3000  // milliseconds until cycle repeat
#define BLINK_DURATION 100 // milliseconds LED is on for

#define MESH_SSID "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

// Prototypes
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendMessage();                                                // Prototype
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage); // start with a one second interval

// Task to blink the number of nodes
void functionBlinkNoNodes();
Task blinkNoNodes(TASK_SECOND * 1, TASK_FOREVER, &functionBlinkNoNodes);
bool onFlag = false;

// Task Scan Networks
void functionScanNetworks();
Task tasksScanNetworks(TASK_SECOND * 10, TASK_FOREVER, &functionScanNetworks);

void setup()
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  mesh.setDebugMsgTypes(ERROR | STARTUP);

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask(tasksScanNetworks);
  tasksScanNetworks.enable();

  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();

  // userScheduler.addTask(blinkNoNodes);
  // blinkNoNodes.enable();
}

void loop()
{
  mesh.update();
  digitalWrite(LED, !onFlag);
}

void functionScanNetworks()
{
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

void functionBlinkNoNodes()
{
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
}
void sendMessage()
{
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  mesh.sendBroadcast(msg);

  if (calc_delay)
  {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end())
    {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  // Serial.printf("Sending message: %s\n", msg.c_str());

  taskSendMessage.setInterval(TASK_SECOND * 5); // between 1 and 5 seconds
}

void receivedCallback(uint32_t from, String &msg)
{
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId)
{
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  // Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  // Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  // nodes = mesh.getNodeList();

  // Serial.printf("Num nodes: %d\n", nodes.size());
  // Serial.printf("Connection list:");

  // SimpleList<uint32_t>::iterator node = nodes.begin();
  // while (node != nodes.end())
  // {
  //   Serial.printf(" %u", *node);
  //   node++;
  // }
  // Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay)
{
  // Serial.printf("Delay to node %u is %d us\n", from, delay);
}
