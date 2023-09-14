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


void setup()
{
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}