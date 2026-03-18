#include <Arduino.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <data_mgr.h>
#include <storage/littlefs_storage.hpp>
#include <esp_log.h>
#include <discovery.h>
#include <mqtt.h>
#include <healthcheck.h>
#include <wirenboard.h>
#include <network/network.h>
#include <log/log.h>
#include <Wire.h>
#include <TCA9555.h>
#include <device/wb_mr6c.h>

#include "defines.h"
#include "config.h"
#include "hallway/command/command_consumer.h"
#include "hallway/hallway.h"
#include "web/handler.h"

TCA9555 tca9555(0x20, &Wire);

EDConfig::DataMgr<Config> configMgr(new EDConfig::StorageLittleFS<Config>("/config.bin"));
EDNetwork::NetworkMgr networkMgr;
EDMQTT::MQTT mqtt;
EDWB::WirenBoard modbus(Serial1, RS485RTS);

EDHealthCheck::HealthCheck healthCheck;
EDHA::DiscoveryMgr discoveryMgr;

Hallway::Hallway hallway(&discoveryMgr, &mqtt, &modbus);
Hallway::CommandConsumer hallwayCommandConsumer(&hallway);

Handler handler(&configMgr, &networkMgr, &healthCheck);

void setup()
{
    randomSeed(micros());

    Serial.begin(SERIAL_SPEED);

    esp_log_level_set("*", ESP_LOG_VERBOSE);

    LOGI("setup", "Alfred");
    LOGI("setup", "start");

    LOGI("setup", "littlefs begin");
    if (!LittleFS.begin(true)) {
        LOGE("setup", "failed to init littlefs");
        return;
    }

    LOGI("setup", "init i2c");
    Wire.begin(I2CSDA, I2CSCL);
    Wire.setClock(100000);

    // tmp enable Vout
    tca9555.begin(OUTPUT, 0x40);

    configMgr.setDefault([](Config* config) {
        snprintf(config->network.wifiAPSSID, WIFI_SSID_LEN, "Alfred_%s", EDUtils::getMacAddress().c_str());
        snprintf(config->mqttHADiscoveryPrefix, MQTT_TOPIC_LEN, "homeassistant");

        strcpy(config->log.host, "192.168.1.2");
        config->log.port = 5555;
        strcpy(config->log.uri, "/_bulk");

        strcpy(config->otaPassword, "somestrongpassword");

        snprintf(config->hallway.mqttStateTopic, MQTT_TOPIC_LEN, "alfred/%s/hallway/state", EDUtils::getChipID());
        snprintf(config->hallway.mqttCommandTopic, MQTT_TOPIC_LEN, "alfred/%s/hallway/set", EDUtils::getChipID());
        config->hallway.modbusAddressMTD262MB = 1;
        config->hallway.modbusAddressWBLED = 2;
        config->hallway.modbusAddressWBMS = 3;

        config->modbusSpeed = 9600;
        config->modbusAddressWBMR6C = 4;
    });
    configMgr.load();

    networkLogger.init(configMgr.getData()->log, CONTROLLER_NAME, EDUtils::formatString("Alfred_%s", EDUtils::getMacAddress().c_str()));

    Serial1.begin(configMgr.getData()->modbusSpeed, SERIAL_8N1, RS485RX, RS485TX);

    modbus.init(15);

    networkMgr.init(configMgr.getData()->network, true, ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);

    ArduinoOTA.setPassword(configMgr.getData()->otaPassword);
    ArduinoOTA.begin();

    mqtt.init(configMgr.getData()->mqtt);
    networkMgr.OnConnect([&](bool isConnected) {
        networkLogger.enable(isConnected);
        LOGD("network", "network event. connected: %s", isConnected ? "true" : "false");

        if (isConnected) {
            mqtt.connect();
        } else {
            mqtt.disconnect();
        }
    });
    healthCheck.registerService(&mqtt);

    handler.init();

    discoveryMgr.init(
        configMgr.getData()->mqttHADiscoveryPrefix,
        configMgr.getData()->mqttIsHADiscovery,
        [](std::string topicName, std::string payload) {
            return mqtt.publish(topicName.c_str(), payload.c_str(), true);
        }
    );

    EDHA::Device* device = discoveryMgr.addDevice();
    device->setHWVersion(deviceHWVersion)
        ->setSWVersion(deviceFWVersion)
        ->setModel(deviceModel)
        ->setName(deviceName)
        ->setManufacturer(deviceManufacturer);

    auto mr6c = modbus.addMR6C(configMgr.getData()->modbusAddressWBMR6C);

    hallway.init(configMgr.getData()->hallway, device, mr6c);

    hallwayCommandConsumer.init(configMgr.getData()->hallway.mqttCommandTopic);
    mqtt.subscribe(&hallwayCommandConsumer);

    LOGI("setup", "complete");
}

void loop()
{
    networkMgr.loop();
    discoveryMgr.loop();
    ArduinoOTA.handle();
    healthCheck.loop();
    networkLogger.update();
    hallway.update();
}
