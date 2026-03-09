#include <ArduinoJSON.h>
#include <FS.h>
#include <LittleFS.h>
#include <esp_system.h>
#include <ExtStrings.h>
#include <Json.h>
#include <cstring>
#include <string.h>
#include "defines.h"
#include "handler.h"

void Handler::init()
{
    _server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    _server->serveStatic("/jquery-3.6.0.min.js", LittleFS, "/jquery-3.6.0.min.js");
    _server->serveStatic("/bootstrap.min.js", LittleFS, "/bootstrap.min.js");

    _healthCheck->registerHandlers(_server);

    _server->on("/api/wifi/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
        /*AsyncResponseStream *response = request->beginResponseStream("application/json");

        DynamicJsonDocument json(200);

        auto networks = this->_wifiService->getWifiNetworks();

        for (auto i = networks.begin(); i != networks.end(); i++) {
            json.add(*i);
        }

        serializeJson(json, *response);

        request->send(response);*/
    });

    _server->on("/api/settings/modbus", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        std::string payload = EDUtils::buildJson([this](JsonObject entity) {
            auto config = _configMgr->getData();

            entity["modbusSpeed"] = config->modbusSpeed;
            entity["modbusAddressWBMR6C"] = config->modbusAddressWBMR6C;
        });

        response->write(payload.c_str());
        request->send(response);
    });

    _server->on("/api/settings/modbus", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (
            !request->hasParam("modbusSpeed", true)
            || !request->hasParam("modbusAddressWBMR6C", true)
        ) {
            request->send(422, "application/json", "{\"message\": \"not present modbus params in request\"}");
            return;
        }

        const AsyncWebParameter* modbusSpeedParam = request->getParam("modbusSpeed", true);
        const AsyncWebParameter* modbusAddressWBMR6CParam = request->getParam("modbusAddressWBMR6C", true);

        int modbusSpeed;
        if (EDUtils::str2int(&modbusSpeed, modbusSpeedParam->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect modbus speed\"}");
            return;
        }

        switch (modbusSpeed) {
            case 1200:
                break;
            case 2400:
                break;
            case 4800:
                break;
            case 9600:
                break;
            case 19200:
                break;
            case 38400:
                break;
            case 57600:
                break;
            case 115200:
                break;
            default:
                request->send(422, "application/json", "{\"message\": \"Unsupported modbus speed\"}");
                return;
        }

        int modbusAddressWBMR6C;
        if (EDUtils::str2int(&modbusAddressWBMR6C, modbusAddressWBMR6CParam->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect WB-MR6C address\"}");
            return;
        }

        _configMgr->getData()->modbusSpeed = modbusSpeed;
        _configMgr->getData()->modbusAddressWBMR6C = modbusAddressWBMR6C;

        if (_configMgr->store()) {
            request->send(200, "application/json", "{}");
        } else {
            request->send(500, "application/json", "{\"message\": \"Failed to update config\"}");
        }
    });

    _server->on("/api/settings/hallway", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        std::string payload = EDUtils::buildJson([this](JsonObject entity) {
            auto config = _configMgr->getData();

            entity["mqttCommandTopic"] = config->hallway.mqttCommandTopic;
            entity["mqttStateTopic"] = config->hallway.mqttStateTopic;

            entity["modbusAddressMTD262MB"] = config->hallway.modbusAddressMTD262MB;
            entity["modbusAddressWBLED"] = config->hallway.modbusAddressWBLED;
            entity["modbusAddressWBMS"] = config->hallway.modbusAddressWBMS;
        });

        response->write(payload.c_str());
        request->send(response);
    });

    _server->on("/api/settings/hallway/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (
            !request->hasParam("mqttCommandTopic", true)
            || !request->hasParam("mqttStateTopic", true)
            || !request->hasParam("modbusAddressMTD262MB", true)
            || !request->hasParam("modbusAddressWBLED", true)
            || !request->hasParam("modbusAddressWBMS", true)
        ) {
            request->send(422, "application/json", "{\"message\": \"not present required fields in request\"}");
            return;
        }

        const AsyncWebParameter* mqttCommandTopicParam = request->getParam("mqttCommandTopic", true);
        const AsyncWebParameter* mqttStateTopicParam = request->getParam("mqttStateTopic", true);
        const AsyncWebParameter* modbusAddressMTD262MBParam = request->getParam("modbusAddressMTD262MB", true);
        const AsyncWebParameter* modbusAddressWBLEDParam = request->getParam("modbusAddressWBLED", true);
        const AsyncWebParameter* modbusAddressWBMSParam = request->getParam("modbusAddressWBMS", true);

        if (mqttCommandTopicParam->value().length() > MQTT_TOPIC_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"WiFi SSID lenght more 64 symbols\"}");
            return;
        }

        if (mqttStateTopicParam->value().length() > MQTT_TOPIC_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"WiFi SSID lenght more 64 symbols\"}");
            return;
        }

        int modbusAddressMTD262MB;
        if (EDUtils::str2int(&modbusAddressMTD262MB, modbusAddressMTD262MBParam->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect MTD262MB address\"}");
            return;
        }

        int modbusAddressWBLED;
        if (EDUtils::str2int(&modbusAddressWBLED, modbusAddressWBLEDParam->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect WB-LED address\"}");
            return;
        }

        int modbusAddressWBMS;
        if (EDUtils::str2int(&modbusAddressWBMS, modbusAddressWBMSParam->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect WB-MS address\"}");
            return;
        }

        std::strcpy(_configMgr->getData()->hallway.mqttCommandTopic, mqttCommandTopicParam->value().c_str());
        std::strcpy(_configMgr->getData()->hallway.mqttStateTopic, mqttStateTopicParam->value().c_str());
        _configMgr->getData()->hallway.modbusAddressMTD262MB = modbusAddressMTD262MB;
        _configMgr->getData()->hallway.modbusAddressWBLED = modbusAddressWBLED;
        _configMgr->getData()->hallway.modbusAddressWBMS = modbusAddressWBMS;

        if (_configMgr->store()) {
            request->send(200, "application/json", "{}");
        } else {
            request->send(500, "application/json", "{\"message\": \"Failed to update config\"}");
        }
    });

    _server->on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        std::string payload = EDUtils::buildJson([this](JsonObject entity) {
            auto config = _configMgr->getData();

            entity["wifiSSID"] = config->network.wifiSSID;
            entity["wifiPassword"] = config->network.wifiPassword;
            entity["mqttHost"] = config->mqtt.host;
            entity["mqttPort"] = config->mqtt.port;
            entity["mqttLogin"] = config->mqtt.login;
            entity["mqttPassword"] = config->mqtt.password;
            entity["mqttIsHADiscovery"] = config->mqttIsHADiscovery;
            entity["mqttHADiscoveryPrefix"] = config->mqttHADiscoveryPrefix;
        });

        response->write(payload.c_str());
        request->send(response);
    });

    _server->on("/api/settings/wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("wifiSSID", true) || !request->hasParam("wifiPassword", true)) {
            request->send(422, "application/json", "{\"message\": \"not present wifiSSID or wifiPassword in request\"}");
            return;
        }

        const AsyncWebParameter* wifiSSID = request->getParam("wifiSSID", true);
        const AsyncWebParameter* wifiPassword = request->getParam("wifiPassword", true);

        if (wifiSSID->value().length() > WIFI_SSID_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"WiFi SSID lenght more 32 symbols\"}");
            return;
        }

        if (wifiPassword->value().length() > WIFI_PWD_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"WiFi password lenght more 63 symbols\"}");
            return;
        }

        auto config = _configMgr->getData();
        std::strcpy(config->network.wifiSSID, wifiSSID->value().c_str());
        std::strcpy(config->network.wifiPassword, wifiPassword->value().c_str());
        config->network.isAPMode = false;

        _configMgr->store();

        request->send(200, "application/json", "{}");
    });

    _server->on("/api/settings/mqtt", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (
            !request->hasParam("host", true)
            || !request->hasParam("port", true)
            || !request->hasParam("login", true)
            || !request->hasParam("password", true)
            || !request->hasParam("haDiscoveryPrefix", true)
        ) {
            request->send(422, "application/json", "{\"message\": \"not present mqtt params in request\"}");
            return;
        }

        auto config = _configMgr->getData();
        const AsyncWebParameter* host = request->getParam("host", true);
        const AsyncWebParameter* port = request->getParam("port", true);
        const AsyncWebParameter* login = request->getParam("login", true);
        const AsyncWebParameter* password = request->getParam("password", true);
        const AsyncWebParameter* haDiscoveryPrefix = request->getParam("haDiscoveryPrefix", true);
        const AsyncWebParameter* ishaDiscoveryEnabled = request->getParam("mqttIsHADiscovery", true);

        if (host->value().length() == 0 || host->value().length() > HOST_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"MQTT host lenght invalid\"}");
            return;
        }

        int mqttPort;
        if (EDUtils::str2int(&mqttPort, port->value().c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            request->send(422, "application/json", "{\"message\": \"Incorrect port\"}");
            return;
        }

        if (login->value().length() > MQTT_LOGIN_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"MQTT login length invalid\"}");
            return;
        }

        if (password->value().length() > MQTT_PASSWORD_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"MQTT password length invalid\"}");
            return;
        }

        if (haDiscoveryPrefix->value().length() == 0) {
            request->send(422, "application/json", "{\"message\": \"You must specify mqtt topic name\"}");
            return;
        }

        if (haDiscoveryPrefix->value().length() > MQTT_TOPIC_LEN-1) {
            request->send(422, "application/json", "{\"message\": \"HA discovery prefix length invalid\"}");
            return;
        }

        strcpy(config->mqtt.host, host->value().c_str());
        config->mqtt.port = (uint16_t)mqttPort;
        strcpy(config->mqtt.login, login->value().c_str());
        strcpy(config->mqtt.password, password->value().c_str());
        strcpy(config->mqttHADiscoveryPrefix, haDiscoveryPrefix->value().c_str());

        if (strcmp(ishaDiscoveryEnabled->value().c_str(), "true") == 0) {
            config->mqttIsHADiscovery = true;
        } else {
            config->mqttIsHADiscovery = false;
        }

        _configMgr->store();

        request->send(200, "application/json", "{}");
    });

    _server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        std::string data = EDUtils::buildJson([this](JsonObject entity) {
            if (_networkMgr->isConnected()) {
                entity[F("wifiStatus")] = "connected";
            } else {
                entity[F("wifiStatus")] = "disconnected";
            }

            entity[F("freeHeap")] = ESP.getFreeHeap();
            entity[F("uptime")] = esp_timer_get_time() / 1000000;

            esp_reset_reason_t reason = esp_reset_reason();

            switch (reason) {
                case ESP_RST_POWERON:
                    entity[F("lastResetReason")] = "Power-on Reset";
                    break;
                case ESP_RST_EXT:
                    entity[F("lastResetReason")] = "External Reset";
                    break;
                case ESP_RST_SW:
                    entity[F("lastResetReason")] = "Software Reset";
                    break;
                case ESP_RST_PANIC:
                    entity[F("lastResetReason")] = "Panic Reset";
                    break;
                case ESP_RST_INT_WDT:
                    entity[F("lastResetReason")] = "Interrupt Watchdog Reset";
                    break;
                case ESP_RST_TASK_WDT:
                    entity[F("lastResetReason")] = "Task Watchdog Reset";
                    break;
                case ESP_RST_WDT:
                    entity[F("lastResetReason")] = "Other Watchdog Reset";
                    break;
                case ESP_RST_DEEPSLEEP:
                    entity[F("lastResetReason")] = "Wake from Deep Sleep";
                    break;
                case ESP_RST_BROWNOUT:
                    entity[F("lastResetReason")] = "Brownout Reset";
                    break;
                case ESP_RST_SDIO:
                    entity[F("lastResetReason")] = "SDIO Reset";
                    break;
                default:
                    entity[F("lastResetReason")] = "Unknown";
                    break;
            }
        });
        response->print(data.c_str());
        request->send(response);
    });

    _server->on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{}");
        delay(200);
        ESP.restart();
    });

    _server->begin();
}
