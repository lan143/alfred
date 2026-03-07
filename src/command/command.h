#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <enum/modes.h>

class Command
{
public:
    bool unmarshalJSON(const char* data);
};
