// Home/main.cpp

//************************************************************
// painlessMesh library - basic.ino
//
// 1. Reads the messages without any repetition
// 2.
//
//************************************************************
#include "painlessMesh.h"
#include "WiFi.h"

#define MESH_PREFIX "myNetwork"
#define MESH_PASSWORD "myPassword"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("%u || %s", from, msg.c_str());
  Serial.println("");
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u", nodeId);
  Serial.println("");
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections %s", mesh.subConnectionJson().c_str());
  Serial.println("");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d", mesh.getNodeTime(), offset);
  Serial.println("");
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
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}