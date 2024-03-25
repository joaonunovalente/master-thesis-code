  /*
  ========================================================================
                  ESP32 Mesh Network Data Transmission
  ========================================================================

  This code facilitates the establishment of a mesh network using ESP32 modules,
  enabling data transmission between nodes.

  Functionalities and Tasks:

  - Configures painlessMesh and WiFi libraries for mesh network communication.
  - Implements periodic broadcast of network scanning data across the mesh.
  - Handles received messages and tracks network connections.
  - Initializes tasks for sending messages and scanning nearby networks.

  Author:   João Nuno Valente
  Email:    jnvalente@ua.pt
  Date:     September, 2023

  ========================================================================
  */

  #include "painlessMesh.h"
  #include "WiFi.h"

  #define MESH_PREFIX "myNetwork"    // Set the mesh network prefix
  #define MESH_PASSWORD "myPassword" // Set the mesh network password
  #define MESH_PORT 5555             // Define the mesh port number

  Scheduler userScheduler; // Scheduler for controlling personal tasks
  painlessMesh mesh;       // Initialize painlessMesh object

  void sendMessage(); // Prototype for sendMessage function

  Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, &sendMessage); // Task to send messages at intervals

  void sendMessage()
  {
    String msg = "Mensagem"; // Define the message to send
    mesh.sendBroadcast(msg); // Send the message to all nodes in the mesh
  }

  void scanNetworks(); // Prototype for scanNetworks function

  Task taskScanNetworks(TASK_SECOND * 10, TASK_FOREVER, &scanNetworks); // Task to scan networks at intervals

  void scanNetworks()
  {
    String msg = ""; // Initialize an empty message

    int numSSID = WiFi.scanNetworks(); // Scan for available networks and get the count

    // Loop through each found network and gather information
    for (int thisNet = 0; thisNet < numSSID; thisNet++)
    {
      msg += "MAC ";                 // Add MAC address to the message
      msg += WiFi.BSSIDstr(thisNet); // Add BSSID (MAC address) of the network
      msg += " | Signal = ";
      msg += WiFi.RSSI(thisNet); // Add signal strength (RSSI) of the network
      msg += " dB";
      msg += " || ";
    }
    mesh.sendBroadcast(msg); // Send the collected network information to all nodes in the mesh
  }

  // Callback function when a message is received
  void receivedCallback(uint32_t from, String &msg)
  {
    Serial.printf("%u | %s\n", from, msg.c_str()); // Print the received message and sender's node ID
  }

  // Callback function for a new connection
  void newConnectionCallback(uint32_t nodeId)
  {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId); // Print when a new connection is established
  }

  // Callback function when connections change
  void changedConnectionCallback()
  {
    Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str()); // Print when mesh connections change
  }

  // Callback function when node time is adjusted
  void nodeTimeAdjustedCallback(int32_t offset)
  {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset); // Print when node time is adjusted
  }

  void setup()
  {
    Serial.begin(115200); // Start serial communication

    mesh.setDebugMsgTypes(ERROR | STARTUP); // Set debug message types for painlessMesh

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT); // Initialize painlessMesh with network details
    mesh.onReceive(&receivedCallback);                                // Set callback for received messages
    mesh.onNewConnection(&newConnectionCallback);                     // Set callback for new connections
    mesh.onChangedConnections(&changedConnectionCallback);            // Set callback for changed connections
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);               // Set callback for adjusted node time

    userScheduler.addTask(taskScanNetworks); // Add the task to scan networks to the user scheduler
    taskScanNetworks.enable();               // Enable the task to scan networks
  }

  void loop()
  {
    mesh.update(); // Update the mesh network, including handling callbacks and tasks in the user scheduler
  }
