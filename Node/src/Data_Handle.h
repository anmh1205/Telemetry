#ifndef DATA_HANDLE_H
#define DATA_HANDLE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Global_Variable.h>
#include <MQTT_Connection.h>

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

class Data_Handle: public MQTT_Connection
{
public:
    void Init();
    void SetNodeName(String name);
    String GetNodeName();
    void SetIdAssign(uint8_t id, String value);
    String GetNameAssign(uint8_t id);
    bool SetDataField(uint8_t id, double value);
    void DataHandleLoop();
    void TestMessageSpeed();
};

#endif // DATA_HANDLE_H