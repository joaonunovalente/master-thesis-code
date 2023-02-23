//************************************************************
// painlessMesh library - basic.ino
//
// 1. scan networks periodically and filter the MAC Addresses
// 2.
//
//
//************************************************************
#include "painlessMesh.h"
#include "WiFi.h"

#define MESH_PREFIX "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

//
void sendMessage(); // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, &sendMessage);

void sendMessage()
{
  String msg = "Hello from";
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
}

//  Function/Task that scan networks
void scanNetworks(); // Prototype so PlatformIO doesn't complain

Task taskScanNetworks(TASK_SECOND * 10, TASK_FOREVER, &scanNetworks);

void scanNetworks()
{
  Serial.println("Scan started");
  // returns the number of networks found
  int numSSID = WiFi.scanNetworks();

  // print the network found:
  for (int thisNet = 0; thisNet < numSSID; thisNet++)
  {
    Serial.print(WiFi.SSID(thisNet));
    Serial.print(" - ");
    Serial.print(WiFi.BSSIDstr(thisNet));
    Serial.print(" | Signal= ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.println("dB");
  }
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("From %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
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

  userScheduler.addTask(taskScanNetworks);
  taskScanNetworks.enable();

  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}
