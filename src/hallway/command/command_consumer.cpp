#include <Arduino.h>
#include <log/log.h>
#include "command_consumer.h"
#include "command.h"

void Hallway::CommandConsumer::consume(std::string payload)
{
    LOGD("command_consumer", "handle");

    if (payload == "stateTerraceLightOn") {
        _hallway->changeFrontTerraceLightState(true);
    } else if (payload == "stateTerraceLightOff") {
        _hallway->changeFrontTerraceLightState(false);
    } else if (payload == "stateHallwayLightOn") {
        _hallway->changeHallwayLightState(true);
    } else if (payload == "stateHallwayLightOff") {
        _hallway->changeHallwayLightState(false);
    } else {
        Command command;
        if (!command.unmarshalJSON(payload.c_str())) {
            LOGE("command_consumer", "cant unmarshal command");
            return;
        }

        if (command.getHallwayLightBrightness().second) {
            _hallway->setHallwayLightBrightness(command.getHallwayLightBrightness().first, true);
        }

        if (command.getHallwayLightTempColor().second) {
            _hallway->setHallwayLightTempColor(command.getHallwayLightTempColor().first);
        }
    }
}
