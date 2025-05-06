#include <Data_Handle.h>

////////////////////////////////////////////////////////////////////////////

String client_name = "default_name";

DynamicJsonDocument data(1024); // Adjust size as needed

DynamicJsonDocument id_assign_doc(1024); // Adjust size as needed
JsonArray id_assign_array;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Data_Handle::Init()
{
    // Initialize the ID assign array ONCE here
    id_assign_array = id_assign_doc.to<JsonArray>();
}

void Data_Handle::SetNodeName(String name)
{
    client_name = name;
}

String Data_Handle::GetNodeName()
{
    return client_name;
}

void Data_Handle::SetIdAssign(uint8_t id, String value)
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

String Data_Handle::GetNameAssign(uint8_t id)
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

bool Data_Handle::SetDataField(uint8_t id, double value)
{
    String value_name = Data_Handle::GetNameAssign(id);
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

void Data_Handle::DataHandleLoop()
{
    if (Serial.available())
    {
        // Read all available data into buffer
        uint8_t buffer[1000];
        uint8_t count = 0;
        while (Serial.available())
        {
            buffer[count++] = Serial.read();
        }

        // Serial.print('R'); // Print the received data for debugging

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
                Data_Handle::SetNodeName(name);
                log("Set Node Name: " + name);

                // Publish the node name
                String json_data;
                serializeJson(data, json_data);
                String topic = Data_Handle::GetNodeName() + "/" + "data";
                MQTT_Connection::Publish(topic.c_str(), json_data.c_str());
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
                    Data_Handle::SetIdAssign(id, value);
                    log("Set ID Assign: " + value + " with ID: " + String(id));

                    // Publish the ID assignment
                    String json_data;
                    serializeJson(id_assign_array, json_data);
                    String topic = Data_Handle::GetNodeName() + "/" + "id_assign";
                    MQTT_Connection::Publish(topic.c_str(), json_data.c_str());
                    // MQTT_PUBLISH_JSON(Data_Handle::GetNodeName() + "/" + "id_assign", json_data);
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

                    if (Data_Handle::SetDataField(id, value))
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
                String topic = Data_Handle::GetNodeName() + "/" + "data";
                MQTT_Connection::Publish(topic.c_str(), json_data.c_str());
                // MQTT_PUBLISH_JSON(Data_Handle::GetNodeName() + "/" + "data", json_data);
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

void Data_Handle::TestMessageSpeed()
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
