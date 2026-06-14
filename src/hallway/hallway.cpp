#include <log/log.h>
#include <binary_sensor/mtd262mb.h>
#include <binary_sensor/wb_mr6c.h>
#include <light/relay_light.h>
#include <light/wb_led_cct.h>
#include <relay/wb_mr6c.h>
#include <sensor/wbms_humidity.h>
#include <sensor/wbms_light_level.h>
#include <sensor/wbms_onewire_temperature.h>
#include <sensor/wbms_temperature.h>

#include "defines.h"
#include "hallway.h"

using namespace Hallway;

void Hallway::Hallway::init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c)
{
    _config = config;
    _mr6c = mr6c;

    _led = _modbus->addLED(config.modbusAddressWBLED);
    _ms = _modbus->addMS(config.modbusAddressWBMS);
    _mtd262mb = _modbus->addMTD262MB(config.modbusAddressMTD262MB);

    if (!_mr6c->setInputMode(MR6C_CHANNEL_ENTRACE_DOOR, EDWB::MR6C_INPUT_MODE_DONT_USE)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_ENTRACE_DOOR");
    }

    if (!_mr6c->setInputMode(MR6C_CHANNEL_TWO, EDWB::MR6C_INPUT_MODE_DONT_USE)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_TWO");
    }

    if (!_mr6c->setInputMode(MR6C_CHANNEL_THREE, EDWB::MR6C_INPUT_MODE_DONT_USE)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_THREE");
    }

    if (!_mr6c->setInputMode(MR6C_CHANNEL_TERRACE_LIGHT_BUTTON, EDWB::MR6C_INPUT_MODE_BUTTON_WITHOUT_LOCKING)) {
        LOGE("init", "failed to set input mode for channel MR6C_CHANNEL_TERRACE_LIGHT_BUTTON");
    }

    _led->setMode(EDWB::LED_MODE_CCTWW);
    _led->setInputMode(1, true);
    _led->setSafeMode(1, EDWB::SAFE_MODE_DONT_BLOCK_INPUT);
    _led->setInputActionRaw(1, EDWB::INPUT_TYPE_SHORT_CLICK, 0x3007); // switch cct1
    _led->setInputActionRaw(1, EDWB::INPUT_TYPE_LONG_CLICK, 0xB008); // change cct1 brightness
    _mr6c->setRelayChannelState(MR6C_RELAY_CHANNEL_POWER_SUPPLY, true); // tmp always enable led power supply

    _temperature = new EDCommon::Sensor::WBMSTemperature(_ms);
    _temperature->init(2, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway temperature"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _humidity = new EDCommon::Sensor::WBMSHumidity(_ms);
    _humidity->init(2, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway humidity"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _floorTemperature = new EDCommon::Sensor::WBMSOneWireTemperature(1, _ms);
    _humidity->init(2, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway floor temperature"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _lightLevel = new EDCommon::Sensor::WBMSLightLevel(_ms);
    _lightLevel->init(0, 10000, {
        EDCommon::Sensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway light level"
        ), EDCommon::Sensor::withDiscovery(_discoveryMgr, device)
    });

    _humanDetector = new EDCommon::BinarySensor::MTD262MB(_mtd262mb);
    _humanDetector->init(500, {
        EDCommon::BinarySensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway human detected"
        ), EDCommon::BinarySensor::withDiscovery(_discoveryMgr, device)
    });

    _entranceDoor = new EDCommon::BinarySensor::WBMR6C(MR6C_CHANNEL_ENTRACE_DOOR, EDHA::deviceClassBinarySensorDoor, true, _mr6c);
    _entranceDoor->init(500, {
        EDCommon::BinarySensor::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Entrance door"
        ), EDCommon::BinarySensor::withDiscovery(_discoveryMgr, device)
    });

    auto hallwayLight = new EDCommon::Light::WBLedCCT(_led);
    hallwayLight->init(1, 1, {
        EDCommon::Light::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Hallway light"
        ), EDCommon::Light::withDiscovery(_discoveryMgr, device)
    });
    _hallwayLight = hallwayLight;

    auto frontTerraceLightRelay = new EDCommon::Relay::WBMR6C(_mr6c);
    frontTerraceLightRelay->init(MR6C_RELAY_CHANNEL_FRONT_TERRACE_LIGHT, {});
    _frontTerraceLight = new EDCommon::Light::Relay(frontTerraceLightRelay);
    _frontTerraceLight->init({
        EDCommon::Light::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred",
            "Front terrace light"
        ), EDCommon::Light::withDiscovery(_discoveryMgr, device)
    });

    _lightAutomation = new EDCommon::Automation::Light(_hallwayLight, nullptr, _humanDetector, _lightLevel);
    _lightAutomation->init("living_room_light_state.bin", {
        EDCommon::Automation::withMQTT(
            _mqtt,
            config.mqttTopicPrefix,
            "alfred"
        ),
        EDCommon::Automation::withDiscovery(_discoveryMgr, device)
    });

    _isInit = true;
}

void Hallway::Hallway::update()
{
    if (!_isInit) {
        return;
    }

    _temperature->update();
    _humidity->update();
    _floorTemperature->update();
    _lightLevel->update();
    _humanDetector->update();
    _entranceDoor->update();
    _frontTerraceLight->update();
    _hallwayLight->update();
    _lightAutomation->update();
}
