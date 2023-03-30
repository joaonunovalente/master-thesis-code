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
  String msg = "Mensagem";
  // msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
}

//  Function/Task that scan networks
void scanNetworks(); // Prototype so PlatformIO doesn't complain

Task taskScanNetworks(TASK_SECOND * 10, TASK_FOREVER, &scanNetworks);

void scanNetworks()
{
  String msg = "";
  
  Serial.println("Scan started");
  // returns the number of networks found
  int numSSID = WiFi.scanNetworks();
  Serial.println("Scan ended");

  // print the network found:
  for (int thisNet = 0; thisNet < numSSID; thisNet++)
  {
    // List of ESP32 MAC addresses
    String MAC_address[] = {
        "78:E3:6D:18:FE:68",
        "78:E3:6D:18:FE:69",
        "4C:11:AE:CA:92:91",
        "78:E3:6D:0A:23:01",
        "78:21:84:8C:B9:39",
        "94:B9:7E:E4:A6:09"};
    // Number of MAC_address
    int Num_MAC_address = sizeof(MAC_address) / sizeof(MAC_address[0]);
    
    for (int ii = 0; ii < Num_MAC_address; ii++)
    {
      // Prints the MAC_addresses and the RSSI
      // that match the ESP32.
      
      if (WiFi.BSSIDstr(thisNet) == MAC_address[ii])
      {
        // Message structure: "MAC 78:E3:6D:18:FE:68 | Signal = -39db"

        msg += "MAC ";
        msg += WiFi.BSSIDstr(thisNet);
        msg += " | Signal = ";
        msg += WiFi.RSSI(thisNet);
        msg += "dB";
        msg += " || ";
      }

      
    }
  }
  mesh.sendBroadcast(msg);
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf(" %u | %s\n", from, msg.c_str());
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