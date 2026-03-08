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
    };
}
