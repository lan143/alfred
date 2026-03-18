#pragma once

#include <device/mtd262mb.h>
#include <device/wb_ms.h>
#include <device/wb_led.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <GyverFilters.h>
#include <mqtt.h>
#include <state/state_mgr.h>
#include <wirenboard.h>

#include "hallway_config.h"
#include "state/producer.h"
#include "state/state.h"

namespace Hallway
{
    class Hallway
    {
    public:
        Hallway(EDHA::DiscoveryMgr* discoveryMgr, EDMQTT::MQTT* mqtt, EDWB::WirenBoard* modbus) : _discoveryMgr(discoveryMgr), _modbus(modbus)
        {
            _temperatureFilter = new GKalman(0.2f, 0.3f);
            _stateProducer = new StateProducer(mqtt);
            _stateMgr = new EDUtils::StateMgr<State>(_stateProducer);
        }

        ~Hallway()
        {
            delete _stateMgr;
            delete _stateProducer;
        }

        void init(Config config, EDHA::Device* device, EDWB::MR6C* mr6c);

        void update();

        void changeFrontTerraceLightState(bool enabled);

    private:
        void buildDiscovery(EDHA::Device* device);
        void updateSensors();

    private:
        Config _config;
        bool _isInit = false;

        uint64_t _lastClimateUpdateTime = 0;
        uint64_t _lastLightUpdateTime = 0;
        uint64_t _lastLightLevelUpdateTime = 0;
        uint64_t _lastHumanDetectorUpdateTime = 0;

        GKalman* _temperatureFilter = nullptr;

    private:
        EDHA::DiscoveryMgr* _discoveryMgr = nullptr;
        StateProducer* _stateProducer = nullptr;
        EDUtils::StateMgr<State>* _stateMgr = nullptr;
        EDWB::WirenBoard* _modbus = nullptr;

        EDWB::LED* _led = nullptr;
        EDWB::MS* _ms = nullptr;
        EDWB::MTD262MB* _mtd262mb = nullptr;
        EDWB::MR6C* _mr6c = nullptr;
    };
}
