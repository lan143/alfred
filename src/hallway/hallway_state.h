#pragma once

#include <Arduino.h>

namespace Hallway
{
    struct StorageState
    {
        bool enabled = false;
        uint8_t brightness = 255;
        uint16_t temperature = 6000;

        bool operator==(StorageState& other)
        {
            return enabled == other.enabled
                && brightness != other.brightness
                && temperature != other.temperature;
        }

        bool operator!=(StorageState& other)
        {
            return !((*this) == other);
        }
    };
}