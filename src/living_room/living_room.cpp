#include "./living_room.h"

using namespace LivingRoom;

void LivingRoom::LivingRoom::init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c)
{
    if (!mr6c->setInputMode(MR6C_CHANNEL_LIVING_ROOM_LIGHT_SWITCH, EDWB::MR6C_INPUT_MODE_BUTTON_WITHOUT_LOCKING)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_FIVE");
    }

    if (!mr6c->setInputMode(MR6C_CHANNEL_SIX, EDWB::MR6C_INPUT_MODE_FREQUENCY)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_SIX");
    }

    auto livingRoomRelay = new EDCommon::Relay::WBMR6C(mr6c);
    livingRoomRelay->init(MR6C_RELAY_CHANNEL_LIVING_ROOM_LIGHT, {});
    _livingRoomLight = new EDCommon::Light::Relay(livingRoomRelay);
    _livingRoomLight->init({
        EDCommon::Light::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room light"
        ),
        EDCommon::Light::withDiscovery(_discoveryMgr, device)
    });

    auto garlandRelay = new EDCommon::Relay::WBMR6C(mr6c);
    garlandRelay->init(MR6C_RELAY_CHANNEL_LIVING_ROOM_GARLAND, {});
    _livingRoomGarland = new EDCommon::Light::Relay(garlandRelay);
    _livingRoomGarland->init({
        EDCommon::Light::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room garland"
        ),
        EDCommon::Light::withDiscovery(_discoveryMgr, device)
    });
}

void LivingRoom::LivingRoom::update()
{
    _livingRoomLight->update();
    _livingRoomGarland->update();
}
