#include <log/log.h>

#include "defines.h"
#include "hallway.h"

using namespace Hallway;

void Hallway::Hallway::init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c)
{
    _config = config;
    _mr6c = mr6c;

    _stateProducer->init(config.mqttStateTopic);

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
    updateLight();

    _stateMgr->loop();
}

void Hallway::Hallway::changeFrontTerraceLightState(bool enabled)
{
    if (!_mr6c->setRelayChannelState(MR6C_CHANNEL_TERRACE_LIGHT_BUTTON, enabled)) {
        LOGE("changeFrontTerraceLightState", "failed to change relay state");
    }

    _stateMgr->getState().changeFrontTerraceLightEnabled(enabled);
}

void Hallway::Hallway::changeHallwayLightState(bool enabled)
{
    changeStateInternal(enabled, true);
}

void Hallway::Hallway::setHallwayLightBrightness(uint8_t brightness, bool updateLight)
{
    _state.brightness = brightness;

    if (updateLight) {
        updateHallwayLight();
    } else {
        _stateMgr->getState().setHallwayLightBrightness(_state.brightness);
    }

    saveState();
}

void Hallway::Hallway::setHallwayLightTempColor(uint16_t tempColor)
{
    _state.temperature = tempColor;

    updateHallwayLight();
    saveState();
}

void Hallway::Hallway::changeStateInternal(bool enabled, bool manual)
{
    if (_state.enabled == enabled) {
        return;
    }

    _state.enabled = enabled;
    _manual = manual;

    if (_manual) {
        _lastManualControlTime = esp_timer_get_time();

        saveState();
    }

    updateHallwayLight();
}

void Hallway::Hallway::updateSensors()
{
    if ((_lastClimateUpdateTime + 10000000) < esp_timer_get_time()) { // every 10 seconds
        auto temperature = _ms->getTemperature();
        if (temperature.second) {
            auto filtered = _temperatureFilter->filtered(temperature.first);
            filtered = std::round(filtered * 100.0f) / 100.0f;
            _stateMgr->getState().setTemperature(filtered);
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
            auto filtered = std::round(floorTemperature.first * 100.0f) / 100.0f;
            _stateMgr->getState().setFloorTemperature(filtered);
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

        auto isDoorClosed = _mr6c->getInputChannelState(MR6C_CHANNEL_ENTRACE_DOOR);
        if (isDoorClosed.second) {
            _stateMgr->getState().changeDoorOpen(!isDoorClosed.first);
        } else {
            LOGE("updateSensors", "failed to get door state");
        }

        _lastHumanDetectorUpdateTime = esp_timer_get_time();
    }

    if ((_lastLightUpdateTime + 500000) < esp_timer_get_time()) { // every 0.5 second
        auto lightEnabled = _mr6c->getRelayChannelState(MR6C_CHANNEL_TERRACE_LIGHT_BUTTON);
        if (lightEnabled.second) {
            _stateMgr->getState().changeFrontTerraceLightEnabled(lightEnabled.first);
        } else {
            LOGE("updateSensors", "failed to get terrace light relay state");
        }

        _lastLightUpdateTime = esp_timer_get_time();
    }
}

void Hallway::Hallway::updateLight()
{
    if ((_lastCheckTime + 500000) < esp_timer_get_time()) {
        auto isEnabled = _led->isEnabledCCT1();
        auto brightness = _led->getBrightnessCCT1();

        if (isEnabled._success && _state.enabled != isEnabled._value) {
            LOGD("automation", "change light state from WB-LED. enabled: %s", isEnabled._value ? "true" : "false");
            changeStateInternal(isEnabled._value, true);
        }

        if (brightness._success && _state.brightness != brightness._value) {
            LOGD("update", "change brightness from WB-LED. brightness: %d", brightness._value);
            setHallwayLightBrightness(brightness._value, false);
        }

        auto isHumanDetected = _stateMgr->getState().isHumanDetected();
        if (isHumanDetected.second && isHumanDetected.first) {
            _lastHumanDetectTime = esp_timer_get_time();
        }

        // enable light if human detected, manual mode isnt active
        if (!_manual && isHumanDetected.second) {
            changeStateInternal(isHumanDetected.first, false);
        }

        _lastCheckTime = esp_timer_get_time();
    }

    if (
        _manual
        && (
            (_state.enabled && (_lastManualControlTime + 300000000) < esp_timer_get_time()) // manual mode turns off after 5 minutes if the light was turned on manually
            || (!_state.enabled && (_lastHumanDetectTime + 600000000) < esp_timer_get_time()) // manual mode turns off 10 minutes after a person leaves the room and light is disabled
        )
    ) {
        _manual = false;
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

    _discoveryMgr->addBinarySensor(
        device,
        "Entrance door",
        "entrance_door",
        EDUtils::formatString("entrance_door_alfred_%s", chipID)
    )
        ->setStateTopic(_config.mqttStateTopic)
        ->setValueTemplate("{{ value_json.isDoorOpen }}")
        ->setPayloadOn("true")
        ->setPayloadOff("false")
        ->setDeviceClass(EDHA::deviceClassBinarySensorDoor);

    _discoveryMgr->addLight(
        device,
        "Front terrace light",
        "frontTerraceLight",
        EDUtils::formatString("front_terrace_light_alfred_%s", EDUtils::getChipID())
    )
        ->setCommandTopic(_config.mqttCommandTopic)
        ->setStateTopic(_config.mqttStateTopic)
        ->setStateValueTemplate("{{ value_json.frontTerraceLightEnabled }}")
        ->setPayloadOn("stateTerraceLightOn")
        ->setPayloadOff("stateTerraceLightOff");

    _discoveryMgr->addLight(
        device,
        "Hallway light",
        "hallwayLight",
        EDUtils::formatString("hallway_light_alfred_%s", EDUtils::getChipID())
    )
        ->setCommandTopic(_config.mqttCommandTopic)
        ->setStateTopic(_config.mqttStateTopic)
        ->setStateValueTemplate("{{ value_json.hallwayLightEnabled }}")
        ->setPayloadOn("stateHallwayLightOn")
        ->setPayloadOff("stateHallwayLightOff")
        ->setBrightnessCommandTopic(_config.mqttCommandTopic)
        ->setBrightnessCommandTemplate("{\"hallwayLightBrightness\": {{ value }} }")
        ->setBrightnessStateTopic(_config.mqttStateTopic)
        ->setBrightnessValueTemplate("{{ value_json.hallwayLightBrightness }}")
        ->setBrightnessScale(100)
        ->setColorTempKelvin(true)
        ->setColorTempCommandTemplate("{\"hallwayLightTempColor\": {{ value }} }")
        ->setColorTempCommandTopic(_config.mqttCommandTopic)
        ->setColorTempStateTopic(_config.mqttStateTopic)
        ->setColorTempValueTemplate("{{ value_json.hallwayLightTempColor }}")
        ->setMinKelvin(2700)
        ->setMaxKelvin(6000);
}

void Hallway::Hallway::updateHallwayLight()
{
    auto result = _led->getTemperatureCCT1();
    auto val = map(_state.temperature, 2700, 6000, 0, 100);

    if (result._success && result._value != val) {
        if (!_led->setTemperatureCCT1(val)) {
            LOGE("updateHallwayLight", "failed to update hallway light temperature");
        }
    } else if (!result._success) {
        LOGE("updateHallwayLight", "failed to get current hallway light temperature");
    }

    if (!_led->setBrightnessCCT1(_state.brightness)) {
        LOGE("updateHallwayLight", "failed to set hallway light brightness");
    }

    if (!_led->enableCCT1(_state.enabled)) {
        LOGE("updateHallwayLight", "failed to update hallway light state");
    }

    _stateMgr->getState().changeHallwayLightEnabled(_state.enabled);
    _stateMgr->getState().setHallwayLightBrightness(_state.brightness);
    _stateMgr->getState().setHallwayLightTempColor(_state.temperature);
}

void Hallway::Hallway::saveState()
{

}
