#pragma once

#include <Arduino.h>

#include "config.h"

namespace Hallway
{
    struct Config
    {
        char mqttCommandTopic[MQTT_TOPIC_LEN] = {0};
        char mqttStateTopic[MQTT_TOPIC_LEN] = {0};

        uint8_t modbusAddressWBMS = 0;
        uint8_t modbusAddressMTD262MB = 0;
        uint8_t modbusAddressWBLED = 0;
    };
}
