#ifndef EVSEMODEL_H
#define EVSEMODEL_H

#include "secc/emFiniteStateMachine.hpp"
#include "secc/ProtocolEVSE.hpp"

#include <map>
#include <functional>
#include <time.h>
#include "emFiniteStateMachine.hpp"
#include "evseInterface.hpp"
#include "FirmwareProxy.hpp"
#include "emSECC.hpp"
#include "SECC_SPI.hpp"


class EVSEModel: public EMSECC
{
private:
    struct tm sysTimeinfo = { 0 };
    EVSE_Interfacer *emEVSE;
public:
    EVSEModel(SECC_SPIClass *pCommIF);
    EVSEModel();
    ~EVSEModel();

    const char * getFirmwareVersion();

    double getAmps();
    double getVoltage();
    double getTotalEnergy();
    double getSessionEnergy();

    bool isVehicleConnected();
    bool isCharging();
    bool isActive();

    uint8_t getEvseState();
};


#endif
