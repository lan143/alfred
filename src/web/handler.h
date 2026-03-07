#pragma once

#ifdef ESP32
    #include <AsyncTCP.h>
#elif defined(ESP8266)
    #include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <data_mgr.h>
#include <healthcheck.h>
#include <network/network.h>

#include "config.h"
class Handler {
public:
    Handler(
        EDConfig::DataMgr<Config>* configMgr,
        EDNetwork::NetworkMgr* networkMgr,
        EDHealthCheck::HealthCheck* healthCheck
    ) : _configMgr(configMgr), _networkMgr(networkMgr),
        _healthCheck(healthCheck) {
        _server = new AsyncWebServer(80);
    }

    void init();

private:
    AsyncWebServer* _server;
    EDConfig::DataMgr<Config>* _configMgr;
    EDNetwork::NetworkMgr* _networkMgr;
    EDHealthCheck::HealthCheck* _healthCheck;
};
