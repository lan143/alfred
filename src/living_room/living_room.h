#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>
#include <light/relay_light.h>
#include <relay/wb_mr6c.h>
#include <wirenboard.h>

#include "./config.h"

namespace LivingRoom
{
    class LivingRoom
    {
    public:
        LivingRoom(
            EDHA::DiscoveryMgr* discoveryMgr,
            EDMQTT::MQTT* mqtt,
            EDWB::WirenBoard* modbus
        ) : _discoveryMgr(discoveryMgr), _mqtt(mqtt), _modbus(modbus) { }

        void init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c);
        void update();

    private:
        EDHA::DiscoveryMgr* _discoveryMgr = nullptr;
        EDMQTT::MQTT* _mqtt = nullptr;
        EDWB::WirenBoard* _modbus = nullptr;

        EDCommon::Light::Relay* _livingRoomLight = nullptr;
        EDCommon::Light::Relay* _livingRoomGarland = nullptr;
    };
}
