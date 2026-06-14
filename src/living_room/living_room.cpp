#include <binary_sensor/mtd262mb.h>
#include <sensor/wbmsw_air_quality.h>
#include <sensor/wbmsw_co2.h>
#include <sensor/wbmsw_humidity.h>
#include <sensor/wbmsw_light_level.h>
#include <sensor/wbmsw_temperature.h>

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

    auto msw = _modbus->addMSW(config.modbusAddressWBMSW);
    auto mtd262mb = _modbus->addMTD262MB(config.modbusAddressMTD262MB);

    _temperature = new EDCommon::Sensor::WBMSWTemperature(msw);
    _temperature->init(2, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room temperature"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _humidity = new EDCommon::Sensor::WBMSWHumidity(msw);
    _humidity->init(2, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room humidity"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _airQuality = new EDCommon::Sensor::WBMSWAirQuality(msw);
    _airQuality->init(0, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room air quality"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _co2 = new EDCommon::Sensor::WBMSWCO2(msw);
    _co2->init(0, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room CO2"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _lightLevel = new EDCommon::Sensor::WBMSWLightLevel(msw);
    _lightLevel->init(0, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room light level"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _humanDetector = new EDCommon::BinarySensor::MTD262MB(mtd262mb);
    _humanDetector->init(500, {
        EDCommon::BinarySensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Living room human detected"
        ), EDCommon::BinarySensor::withDiscovery(_discoveryMgr, device)
    });

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

    _lightAutomation = new EDCommon::Automation::Light(_livingRoomLight, nullptr, _humanDetector, _lightLevel);
    _lightAutomation->init("living_room_light_state.bin", {
        EDCommon::Automation::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred"
        ),
        EDCommon::Automation::withDiscovery(_discoveryMgr, device)
    });
}

void LivingRoom::LivingRoom::update()
{
    _livingRoomLight->update();
    _livingRoomGarland->update();
    _temperature->update();
    _humidity->update();
    _airQuality->update();
    _co2->update();
    _lightLevel->update();
    _humanDetector->update();
    _lightAutomation->update();
}
