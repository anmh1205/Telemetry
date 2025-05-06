#include <MQTT_Connection.h>

PicoMQTT::Client mqtt(
    "192.168.5.1", // MQTT broker address
    1883,          // MQTT broker port
    "robot");

void MQTT_Connection::ConnectedCallback()
{
    // log("Connected to MQTT broker");
}

void MQTT_Connection::DisconnectedCallback()
{
    // log("Disconnected from MQTT broker");
}

bool MQTT_Connection::Init()
{
    mqtt.begin();

    mqtt.connected_callback = [this]()
    {
        MQTT_Connection::ConnectedCallback();
    };
    mqtt.disconnected_callback = [this]()
    {
        MQTT_Connection::DisconnectedCallback();
    };

    return true;
}

bool MQTT_Connection::Publish(const char *topic, const char *payload)
{
    return mqtt.publish(topic, payload);
}
bool MQTT_Connection::Subscribe(const char *topic)
{
    return mqtt.subscribe(topic);
}

void MQTT_Connection::Loop()
{
    mqtt.loop();
}
