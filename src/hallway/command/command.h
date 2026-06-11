#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <enum/modes.h>

namespace Hallway
{
    class Command
    {
    public:
        bool unmarshalJSON(const char* data);

        std::pair<uint8_t, bool> getHallwayLightBrightness() const { return _hallwayLightBrightness; }
        std::pair<uint16_t, bool> getHallwayLightTempColor() const { return _hallwayLightTempColor; }

    private:
        std::pair<uint8_t, bool> _hallwayLightBrightness;
        std::pair<uint16_t, bool> _hallwayLightTempColor;
    };
}
