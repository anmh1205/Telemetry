#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <Arduino.h>
#include <PicoMQTT.h>
#include <Global_Variable.h>

#define MQTT_BEGIN() mqtt.begin()
#define MQTT_LOOP() mqtt.loop()
#define MQTT_PUBLISH(topic, payload) mqtt.publish(topic, payload)
#define MQTT_SUBSCRIBE(topic) mqtt.subscribe(topic)
#define MQTT_PUBLISH_JSON(topic, payload) mqtt.publish(topic, payload.c_str())

class MQTT_Connection
{
public:
    bool Init();
    bool Publish(const char *topic, const char *payload);
    bool Subscribe(const char *topic);
    void Loop();
    void ConnectedCallback();
    void DisconnectedCallback();
};

#endif // MQTT_CONNECTION_H
