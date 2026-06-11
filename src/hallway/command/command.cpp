#include <ArduinoJson.h>
#include <Json.h>
#include <ExtStrings.h>
#include "defines.h"
#include "command.h"

bool Hallway::Command::unmarshalJSON(const char* data)
{
    return EDUtils::parseJson(data, [this](JsonObject root) {
        if (root.containsKey(F("hallwayLightBrightness"))) {
            _hallwayLightBrightness = std::make_pair(root[F("hallwayLightBrightness")].as<uint8_t>(), true);
        }

        if (root.containsKey(F("hallwayLightTempColor"))) {
            _hallwayLightTempColor = std::make_pair(root[F("hallwayLightTempColor")].as<uint16_t>(), true);
        }

        return true;
    });
}
