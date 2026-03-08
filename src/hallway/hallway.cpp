#include "hallway.h"

using namespace Hallway;

void Hallway::Hallway::init(Config config, EDHA::Device* device)
{
    _config = config;

    _stateProducer->init(config.mqttStateTopic);

    _led = _modbus->addLED(config.modbusAddressWBLED);
    _ms = _modbus->addMS(config.modbusAddressWBMS);
    _mtd262mb = _modbus->addMTD262MB(config.modbusAddressMTD262MB);

    buildDiscovery(device);

    _isInit = true;
}

void Hallway::Hallway::update()
{
    if (!_isInit) {
        LOGE("hallway", "failed to run update: hallway isn't initialized");
        return;
    }

    updateSensors();
    _stateMgr->loop();
}

void Hallway::Hallway::updateSensors()
{
    if ((_lastClimateUpdateTime + 10000000) < esp_timer_get_time()) { // every 10 seconds
        auto temperature = _ms->getTemperature();
        if (temperature.second) {
            _stateMgr->getState().setTemperature(temperature.first);
        } else {
            LOGE("updateSensors", "failed to get temperature");
        }

        auto humidity = _ms->getHumidity();
        if (humidity.second) {
            _stateMgr->getState().setHumidity(humidity.first);
        } else {
            LOGE("updateSensors", "failed to get humidity");
        }

        auto floorTemperature = _ms->getOneWireTemperature(1);
        if (floorTemperature.second) {
            _stateMgr->getState().setFloorTemperature(floorTemperature.first);
        } else {
            LOGE("updateSensors", "failed to get one wire sensor temperature");
        }

        _lastClimateUpdateTime = esp_timer_get_time();
    }

    if ((_lastLightLevelUpdateTime + 1000000) < esp_timer_get_time()) { // every second
        auto lightLevel = _ms->getLightLevel();
        if (lightLevel.second) {
            _stateMgr->getState().setLightLevel(lightLevel.first);
        } else {
            LOGE("updateSensors", "failed to get light level");
        }

        _lastLightLevelUpdateTime = esp_timer_get_time();
    }

    if ((_lastHumanDetectorUpdateTime + 500000) < esp_timer_get_time()) { // every 0.5 second
        auto humanDetected = _mtd262mb->isHumanDetected();
        if (humanDetected.second) {
            _stateMgr->getState().changeHumanDetected(humanDetected.first);
        } else {
            LOGE("updateSensors", "failed to get occupancy state");
        }

        _lastHumanDetectorUpdateTime = esp_timer_get_time();
    }
}

void Hallway::Hallway::buildDiscovery(EDHA::Device* device)
{
    const char* chipID = EDUtils::getChipID();

    _discoveryMgr->addSensor(
        device,
        "Temperature",
        "temperature",
        EDUtils::formatString("temperature_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.temperature }}")
        ->setUnitOfMeasurement("°C")
        ->setDeviceClass(EDHA::deviceClassSensorTemperature);

    _discoveryMgr->addSensor(
        device,
        "Humidity",
        "humidity",
        EDUtils::formatString("humidity_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.humidity }}")
        ->setUnitOfMeasurement("%")
        ->setDeviceClass(EDHA::deviceClassSensorHumidity);

    _discoveryMgr->addSensor(
        device,
        "Floor temperature",
        "floorTemperature",
        EDUtils::formatString("floor_temperature_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.floorTemperature }}")
        ->setUnitOfMeasurement("°C")
        ->setDeviceClass(EDHA::deviceClassSensorTemperature);

    _discoveryMgr->addSensor(
        device,
        "Light level",
        "lightLevel",
        EDUtils::formatString("light_level_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.lightLevel }}")
        ->setUnitOfMeasurement("lx")
        ->setDeviceClass("illuminance");

    _discoveryMgr->addBinarySensor(
        device,
        "Human detected",
        "human_detected",
        EDUtils::formatString("human_detected_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.isHumanDetected }}")
        ->setPayloadOn("true")
        ->setPayloadOff("false")
        ->setDeviceClass("occupancy");
}
