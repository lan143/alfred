#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace Hallway
{
    class State
    {
    public:
        State() {}

        bool operator==(State& other);
        bool operator!=(State& other) { return !(*this == other); }

        std::string marshalJSON();

        void setTemperature(float_t temperature) { _temperature = std::make_pair(temperature, true); }
        void setHumidity(float_t humidity) { _humidity = std::make_pair(humidity, true); }
        void setFloorTemperature(float_t floorTemperature) { _floorTemperature = std::make_pair(floorTemperature, true); }

        void setLightLevel(uint16_t lightLevel) { _lightLevel = std::make_pair(lightLevel, true); }
        void changeHumanDetected(bool detected) { _isHumanDetected = std::make_pair(detected, true); }
        void changeNightModeActive(bool active) { _isNightModeActive = std::make_pair(active, true); }
        void changeDoorOpen(bool open) { _isDoorOpen = std::make_pair(open, true); }

    private:
        std::pair<float_t, bool> _temperature;
        std::pair<float_t, bool> _humidity;
        std::pair<float_t, bool> _floorTemperature;

        std::pair<uint16_t, bool> _lightLevel;
        std::pair<bool, bool> _isHumanDetected;
        std::pair<bool, bool> _isNightModeActive;
        std::pair<bool, bool> _isDoorOpen;
    };
}
