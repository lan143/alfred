#include <Arduino.h>
#include <log/log.h>
#include "command_consumer.h"
#include "command.h"

void Hallway::CommandConsumer::consume(std::string payload)
{
    LOGD("command_consumer", "handle");

    Command command;
    if (!command.unmarshalJSON(payload.c_str())) {
        LOGE("command_consumer", "cant unmarshal command");
        return;
    }
}
