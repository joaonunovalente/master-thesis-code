// main.cpp
// *************************************
//                #include
// *************************************
#include <Arduino.h>
#include <WiFi.h>
#include <painlessMesh.h>

// Replace with your network credentials (STATION)
const char *ssid = "myNetwork";
const char *password = "myPassword";

void setup()
{
  // Intro setup()
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
}

void loop()
{
  // *************************************
  //                WiFi
  // *************************************

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