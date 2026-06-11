#pragma once

#include <Arduino.h>

#include "defines.h"

namespace LivingRoom
{
    struct Config
    {
        char mqttTopicPrefix[MQTT_TOPIC_LEN] = {0};

        uint8_t modbusAddressMTD262MB = 0;
        uint8_t modbusAddressWBMSW = 0;
    };
}
