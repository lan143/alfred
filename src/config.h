#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <mqtt_config.h>
#include <network/network_config.h>
#include <log/log_config.h>

#include "defines.h"
#include "hallway/config.h"
#include "living_room/config.h"

#define CURRENT_VERSION 3

struct Config
{
    uint8_t version = CURRENT_VERSION;

    EDNetwork::Config network;
    EDMQTT::Config mqtt;
    EDUtils::LogConfig log;

    char otaPassword[WIFI_PWD_LEN] = {0};

    bool mqttIsHADiscovery = true;
    char mqttHADiscoveryPrefix[MQTT_TOPIC_LEN] = {0};

    // modbus
    uint32_t modbusSpeed = 0;
    uint8_t modbusAddressWBMR6C = 0;

    Hallway::Config hallway;
    LivingRoom::Config livingRoom;
};
