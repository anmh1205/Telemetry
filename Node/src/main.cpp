#include <Arduino.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include <WiFi.h>


///////////////////////////////////////////////////////////////////////////////////
// Define pin and LED count
#define LED_PIN 33
#define LED_COUNT 1

////////////////////////////////////////////////////////////////////////////////////

char ssid[] = "AML_Robocon";
char pass[] = "aml305b4";

/////////////////////////////////////////////////////////////////////////////////
// #define log(x) Serial.println(x)
#define log(x) NULL // Disable logging for production

/////////////////////////////////////////////////////////////////////////////////

PicoMQTT::Client mqtt(
    "192.168.5.1", // MQTT broker address
    1883,          // MQTT broker port
    "robot");

#define MQTT_BEGIN() mqtt.begin()
#define MQTT_LOOP() mqtt.loop()
#define MQTT_PUBLISH(topic, payload) mqtt.publish(topic, payload)
// #define MQTT_SUBSCRIBE(topic) mqtt.subscribe(topic)
#define MQTT_PUBLISH_JSON(topic, payload) mqtt.publish(topic, payload.c_str())

////////////////////////////////////////////////////////////////////////////////
// Define command bytes

#define HEADER_BYTE 0xFF
#define SET_NODE_NAME_COMMAND 0x51
#define SET_ID_ASSIGN_COMMAND 0x52
#define SET_DATA_FIELD_COMMAND 0x53
#define PUBLISH_DATA_COMMAND 0x54
#define HEADER buffer[0]
#define COMMAND buffer[1]
#define ID buffer[2]

#define MAX_NUM_COMMANDS 30

////////////////////////////////////////////////////////////////////////////

String client_name = "robot";

DynamicJsonDocument data(1024); // Adjust size as needed

DynamicJsonDocument id_assign_doc(1024); // Adjust size as needed
JsonArray id_assign_array;

/////////////////////////////////////////////////////////////////////////////

unsigned long previousMillis = 0;
unsigned long previousMillis_1K = 0;
const long interval = 2; // Interval to check for messages

unsigned long count = 0;
unsigned long count_1K = 0;
unsigned long previousCount = 0;

unsigned long success_count = 0;
unsigned long fail_count = 0;

String send_string = "";

/////////////////////////////////////////////////////////////////////////////
void SetNodeName(String name);
void SetIdAssign(uint8_t id, String value);
String GetNameAssign(uint8_t id);
bool SetDataField(uint8_t id, double value);
void DataHandleLoop();
void TestMessageSpeed();
/////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(230400);
  Serial2.begin(230400);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    log("Connecting to WiFi...");
    delay(200);
  }

  log("Connected to WiFi");

  // Initialize the ID assign array ONCE here
  id_assign_array = id_assign_doc.to<JsonArray>();

  MQTT_BEGIN();
}

void loop()
{
  MQTT_LOOP();
  mqtt.loop();

  DataHandleLoop();

  // TestMessageSpeed();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetNodeName(String name)
{
  client_name = name;
}

String GetNodeName()
{
  return client_name;
}

void SetIdAssign(uint8_t id, String value)
{
  // Check if id already exists in array
  for (int i = 0; i < id_assign_array.size(); i++)
  {
    JsonObject obj = id_assign_array[i];
    // Check if this object has the id we're looking for
    if (obj["id"] == id)
    {
      // Update existing value
      obj["value"] = value;

      data[value] = 0; // Initialize the data field with 0

      return;
    }
  }

  // If not found, create a new object in the array
  JsonObject obj = id_assign_array.createNestedObject();
  obj["id"] = id;
  obj["value"] = value;
  data[value] = 0; // Initialize the data field with 0
}

String GetNameAssign(uint8_t id)
{
  for (int i = 0; i < id_assign_array.size(); i++)
  {
    JsonObject obj = id_assign_array[i];
    if (obj["id"] == id)
    {
      return obj["value"].as<String>();
    }
  }

  return "";
}

bool SetDataField(uint8_t id, double value)
{
  String value_name = GetNameAssign(id);
  if (value_name == "")
  {
    return false;
  }

  // Check if the value_name already exists in the data object
  if (data.containsKey(value_name))
  {
    // If it exists, update the value
    data[value_name] = value;
    log("Updated " + value_name + " to " + String(value));
  }
  else
  {
    // If it doesn't exist, create a new key-value pair
    data[value_name] = value;
    log("Created " + value_name + " with value " + String(value));
  }

  return true;
}

void DataHandleLoop()
{
  if (Serial2.available())
  {
    // Read all available data into buffer
    uint8_t buffer[1000];
    uint8_t count = 0;
    while (Serial2.available())
    {
      buffer[count++] = Serial2.read();
    }

    // First pass: scan buffer and mark all VALID command positions
    uint8_t headerIndices[MAX_NUM_COMMANDS]; // maximun command positions
    uint8_t headerCount = 0;

    for (uint8_t i = 0; i < count - 1; i++) // Note the -1 to ensure we can check i+1
    {
      // Check for header byte followed by a valid command byte
      if (buffer[i] == HEADER_BYTE && headerCount < MAX_NUM_COMMANDS)
      {
        uint8_t commandByte = buffer[i + 1];

        // Only accept valid command bytes
        if (commandByte == SET_NODE_NAME_COMMAND ||
            commandByte == SET_ID_ASSIGN_COMMAND ||
            commandByte == SET_DATA_FIELD_COMMAND ||
            commandByte == PUBLISH_DATA_COMMAND)
        {
          headerIndices[headerCount++] = i;
        }
      }
    }

    // Second pass: process each command
    for (uint8_t i = 0; i < headerCount; i++)
    {
      uint8_t startIdx = headerIndices[i];
      uint8_t endIdx = (i < headerCount - 1) ? headerIndices[i + 1] : count;

      // We already validated the command byte, so we can use it directly
      uint8_t cmd = buffer[startIdx + 1];

      switch (cmd)
      {
      case SET_NODE_NAME_COMMAND:
      {
        String name;
        for (uint8_t j = startIdx + 2; j < endIdx; j++)
        {
          name += (char)buffer[j];
        }
        SetNodeName(name);
        log("Set Node Name: " + name);

        // Publish the node name
        String json_data;
        serializeJson(data, json_data);
        MQTT_PUBLISH_JSON(GetNodeName() + "/" + "data", json_data);
      }
      break;

      case SET_ID_ASSIGN_COMMAND:
      {
        // Make sure we have ID byte
        if (startIdx + 2 < count)
        {
          uint8_t id = buffer[startIdx + 2];
          String value;
          for (uint8_t j = startIdx + 3; j < endIdx; j++)
          {
            value += (char)buffer[j];
          }
          SetIdAssign(id, value);
          log("Set ID Assign: " + value + " with ID: " + String(id));

          // Publish the ID assignment
          String json_data;
          serializeJson(id_assign_array, json_data);
          MQTT_PUBLISH_JSON(GetNodeName() + "/" + "id_assign", json_data);
          log("Publish ID Assign: " + json_data);
        }
      }
      break;

      case SET_DATA_FIELD_COMMAND:
      {
        // Make sure we have ID byte
        if (startIdx + 2 < count)
        {
          uint8_t id = buffer[startIdx + 2];

          double value = 0.0;
          memcpy(&value, &buffer[startIdx + 3], sizeof(double)); // Read the double value from buffer

          if (SetDataField(id, value))
          {
            log("Set Data Field: " + String(value) + " with ID: " + String(id));
          }
          else
          {
            log("Failed to set data field for ID: " + String(id));
          }
        }
      }
      break;

      case PUBLISH_DATA_COMMAND:
      {
        String json_data;
        serializeJson(data, json_data);
        MQTT_PUBLISH_JSON(GetNodeName() + "/" + "data", json_data);
        log("Publish Data: " + json_data);
      }
      break;

      default:
        log("Unknown command: " + String(cmd));
        break;
      }
    }
  }
}

void TestMessageSpeed()
{
  // if (millis() - previousMillis >= interval)
  // {
  //   previousMillis = millis();

  //   // Use a literal C-string instead of converting
  //   const char *test_topic = "rbc/test_message";
  //   const char *test_payload = "0123456789012345678901234567890123456789";

  //   if (mqtt.publish(Topic(test_topic), test_payload))
  //   {
  //     success_count++;
  //   }
  //   else
  //   {
  //     fail_count++;
  //   }

  //   count++;
  // }

  // if (millis() - previousMillis_1K >= 1000)
  // {
  //   previousMillis_1K = millis();

  //   String countStr = String(count - previousCount);
  //   String successStr = String(success_count);
  //   String failStr = String(fail_count);

  //   mqtt.publish(Topic("test/message_per_second"), countStr.c_str());
  //   mqtt.publish(Topic("test/success"), successStr.c_str());
  //   mqtt.publish(Topic("test/fail"), failStr.c_str());

  //   previousCount = count;
  // }
}