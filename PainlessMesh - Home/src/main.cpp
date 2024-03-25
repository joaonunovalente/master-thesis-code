/*
============================================================================
                 ESP32 Mesh Network Data Transmission
============================================================================

This code initializes a mesh network using ESP32 modules, enabling data
transmission between nodes.

Functionalities and Tasks:

- Defines the mesh network credentials and sets up painlessMesh and WiFi
  libraries for communication.
- Defines callback functions for handling received messages, new connections,
  changed connections, and adjusted node time.
- Initializes the mesh network and sets up callbacks for network events.
- Manages the update of the mesh network functionality in the main loop.

Author:   João Nuno Valente
Email:    jnvalente@ua.pt
Date:     September, 2023

========================================================================
*/
#include "painlessMesh.h"
#include "WiFi.h"

#define MESH_PREFIX "myNetwork"    // Network name
#define MESH_PASSWORD "myPassword" // Network password
#define MESH_PORT 5555             // Communication port

Scheduler userScheduler; // Scheduler for personal tasks
painlessMesh mesh;       // PainlessMesh object for mesh networking

// Callback function when a message is received
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("%u || %s", from, msg.c_str()); // Print sender's ID and message
  Serial.println("");
}

// Callback function when a new connection is established
void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u", nodeId);
  Serial.println("");
}

// Callback function when the connections in the mesh network change
void changedConnectionCallback()
{
  Serial.printf("Changed connections %s", mesh.subConnectionJson().c_str());
  Serial.println("");
}

// Callback function when the node's time is adjusted
void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d", mesh.getNodeTime(), offset);
  Serial.println("");
}

void setup()
{
  Serial.begin(115200); // Initialize serial communication

  mesh.setDebugMsgTypes(ERROR | STARTUP); // Set debug message types

  // Initialize the mesh network with given credentials and settings
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop()
{
  mesh.update(); // Update mesh networking functionality
}
