// main.cpp
// *************************************
//                #include
// *************************************
#include <Arduino.h>
#include <WiFi.h>

// Replace with your network credentials (AP)
const char *ssid = "myNetwork";
const char *password = "myPassword";

void setup()
{
  // Intro setup()
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
}

void loop()
{
}