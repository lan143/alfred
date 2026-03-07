#pragma once

#include <Arduino.h>
#include <mqtt.h>
#include <producer.h>
#include <state/state_producer.h>
#include "state.h"

class StateProducer : public EDMQTT::Producer, public EDUtils::StateProducer<State>
{
public:
    StateProducer(EDMQTT::MQTT* mqtt) : Producer(mqtt) {}

    bool publish(State* state);
};
