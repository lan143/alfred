#include "producer.h"

bool Hallway::StateProducer::publish(State* state)
{
    if (!_mqtt->isConnected()) {
        return false;
    }

    std::string payload = state->marshalJSON();
    return _mqtt->publish(_topic, payload.c_str(), false);
}
