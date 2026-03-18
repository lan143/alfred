#pragma once

#include <Arduino.h>
#include <consumer.h>

#include "hallway/hallway.h"

namespace Hallway
{
    class CommandConsumer : public EDMQTT::Consumer
    {
    public:
        CommandConsumer(Hallway* hallway) : _hallway(hallway) {}
        void consume(std::string payload);

    private:
        Hallway* _hallway = nullptr;
    };
}
