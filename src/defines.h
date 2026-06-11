#pragma once

#define SERIAL_SPEED 115200

#define ETH_ADDR        0
#define ETH_POWER_PIN  5
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN  18
#define ETH_TYPE      ETH_PHY_RTL8201
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

#define RELAY_WARM_FLOOR 2
#define RELAY_UNKNOW 15

#define RS485RX  9
#define RS485TX  10
#define RS485RTS 4

#define I2CSDA 32
#define I2CSCL 33

#define MR6C_CHANNEL_ENTRACE_DOOR              1
#define MR6C_CHANNEL_TWO                       2
#define MR6C_CHANNEL_THREE                     3
#define MR6C_CHANNEL_TERRACE_LIGHT_BUTTON      4
#define MR6C_CHANNEL_LIVING_ROOM_LIGHT_SWITCH  5
#define MR6C_CHANNEL_SIX                       6

#define MR6C_RELAY_CHANNEL_POWER_SUPPLY        1
#define MR6C_RELAY_CHANNEL_FRONT_TERRACE_LIGHT 4
#define MR6C_RELAY_CHANNEL_LIVING_ROOM_LIGHT   5
#define MR6C_RELAY_CHANNEL_LIVING_ROOM_GARLAND 6

#ifndef CONTROLLER_NAME
#define CONTROLLER_NAME "Alfred"
#endif

#define WIFI_SSID_LEN 32 + 1
#define WIFI_PWD_LEN 64 + 1

#define HOST_LEN 64
#define MQTT_DEFAULT_PORT 1883

#define MQTT_LOGIN_LEN 32
#define MQTT_PASSWORD_LEN 32
#define MQTT_TOPIC_LEN 64


const char deviceName[] = CONTROLLER_NAME;
const char deviceModel[] = "WB-MGE";
const char deviceManufacturer[] = "WirenBoard";
const char deviceHWVersion[] = "v.3";
const char deviceFWVersion[] = "0.1.0";
