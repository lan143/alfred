#pragma once

#include <Arduino.h>
#include <consumer.h>

namespace Hallway
{
    class CommandConsumer : public EDMQTT::Consumer
    {
    public:
        CommandConsumer() {}
        void consume(std::string payload);
    };
}
