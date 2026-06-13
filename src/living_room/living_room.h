#pragma once

#include <Arduino.h>
#include <binary_sensor/binary_sensor.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>
#include <light/relay_light.h>
#include <relay/wb_mr6c.h>
#include <sensor/sensor.h>
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

        EDCommon::BinarySensor::BinarySensor* _humanDetector = nullptr;
        EDCommon::Sensor::Sensor* _temperature = nullptr;
        EDCommon::Sensor::Sensor* _humidity = nullptr;
        EDCommon::Sensor::Sensor* _airQuality = nullptr;
        EDCommon::Sensor::Sensor* _co2 = nullptr;
        EDCommon::Sensor::Sensor* _lightLevel = nullptr;
    };
}
