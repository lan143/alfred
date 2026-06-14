#pragma once

#include <automation/light.h>
#include <binary_sensor/binary_sensor.h>
#include <device/mtd262mb.h>
#include <device/wb_ms.h>
#include <device/wb_led.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>
#include <light/light.h>
#include <state/state_mgr.h>
#include <sensor/sensor.h>
#include <wirenboard.h>

#include "./config.h"

namespace Hallway
{
    class Hallway
    {
    public:
        Hallway(EDHA::DiscoveryMgr* discoveryMgr, EDMQTT::MQTT* mqtt, EDWB::WirenBoard* modbus) : _discoveryMgr(discoveryMgr), _modbus(modbus) { }

        void init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c);
        void update();

    private:
        Config _config;
        bool _isInit = false;

    private:
        EDHA::DiscoveryMgr* _discoveryMgr = nullptr;
        EDMQTT::MQTT* _mqtt = nullptr;
        EDWB::WirenBoard* _modbus = nullptr;

        EDWB::LED* _led = nullptr;
        EDWB::MS* _ms = nullptr;
        EDWB::MTD262MB* _mtd262mb = nullptr;
        EDWB::MR6C* _mr6c = nullptr;

        EDCommon::Sensor::Sensor* _temperature = nullptr;
        EDCommon::Sensor::Sensor* _humidity = nullptr;
        EDCommon::Sensor::Sensor* _floorTemperature = nullptr;
        EDCommon::Sensor::Sensor* _lightLevel = nullptr;
        EDCommon::BinarySensor::BinarySensor* _humanDetector = nullptr;
        EDCommon::BinarySensor::BinarySensor* _entranceDoor = nullptr;
        EDCommon::Light::Light* _frontTerraceLight = nullptr;
        EDCommon::Light::Light* _hallwayLight = nullptr;

        EDCommon::Automation::Light* _lightAutomation = nullptr;
    };
}
