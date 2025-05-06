#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <Global_Variable.h>
#include <MQTT_Connection.h>
#include <Data_Handle.h>

///////////////////////////////////////////////////////////////////////////////////
// Define pin and LED count
#define LED_PIN 33
#define LED_COUNT 1

////////////////////////////////////////////////////////////////////////////////////

char ssid[] = "AML_Robocon";
char pass[] = "aml305b4";

MQTT_Connection mqtt_connection;
Data_Handle data_handle;

////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    log("Connecting to WiFi...");
    delay(200);
  }
  log("Connected to WiFi");

  mqtt_connection.Init();
  data_handle.Init();
  
  log("MQTT connection initialized");
}

void loop()
{
  mqtt_connection.Loop();
  data_handle.DataHandleLoop();
}
