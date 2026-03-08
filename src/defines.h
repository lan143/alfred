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

#define EEPROM_SIZE 2048

#ifndef CONTROLLER_NAME
#define CONTROLLER_NAME "Alfred"
#endif

const char deviceName[] = CONTROLLER_NAME;
const char deviceModel[] = "WB-MGE";
const char deviceManufacturer[] = "WirenBoard";
const char deviceHWVersion[] = "v.3";
const char deviceFWVersion[] = "0.1.0";
