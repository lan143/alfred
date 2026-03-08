#include <Json.h>
#include <ExtStrings.h>
#include "defines.h"
#include "state.h"

using namespace Hallway;

bool State::operator==(State& other)
{
    return _temperature == other._temperature
        && _humidity == other._humidity
        && _floorTemperature == other._floorTemperature
        && _lightLevel == other._lightLevel
        && _isHumanDetected == other._isHumanDetected
        && _isNightModeActive == other._isNightModeActive;
}

std::string State::marshalJSON()
{
    std::string payload = EDUtils::buildJson([this](JsonObject entity) {
        if (_temperature.second) {
            entity[F("temperature")] = _temperature.first;
        }

        if (_humidity.second) {
            entity[F("humidity")] = _humidity.first;
        }

        if (_floorTemperature.second) {
            entity[F("floorTemperature")] = _floorTemperature.first;
        }

        if (_lightLevel.second) {
            entity[F("lightLevel")] = _lightLevel.first;
        }

        if (_isHumanDetected.second) {
            entity[F("isHumanDetected")] = _isHumanDetected.first ? "true" : "false";
        }

        if (_isNightModeActive.second) {
            entity[F("isNightModeActive")] = _isNightModeActive.first ? "true" : "false";
        }
    });

    return payload;
}
