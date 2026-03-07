#pragma once

#include <Arduino.h>
#include <consumer.h>

class CommandConsumer : public EDMQTT::Consumer
{
public:
    CommandConsumer() {}
    void consume(std::string payload);
};
