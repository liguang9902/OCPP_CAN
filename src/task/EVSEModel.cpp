
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <FS.h>
#include <core/NewOCPP.h>
#include <ArduinoOcpp/Tasks/SmartCharging/SmartChargingService.h>

#include "task/EVSEModel.h"
#include <esp_log.h>
#include "emFiniteStateMachine.hpp"
#include "ProtocolEVSE.hpp"
#include "evseInterface.hpp"
#include "emSECC.hpp"
#include "emChargePoint.hpp"
#include "FirmwareProxy.hpp"
//extern void esp_sync_sntp(struct tm& sysTimeinfo , int retryMax);
#include "Common.hpp"
#include "SECC_SPI.hpp"

EVSEModel::EVSEModel(SECC_SPIClass *pCommIF):EMSECC(pCommIF)
{   
    esp_sync_sntp(sysTimeinfo, 5);
    emEVSE = new EVSE_Interfacer(pCommIF);
}

EVSEModel::EVSEModel(){
    
}

EVSEModel::~EVSEModel()
{
}

const char * EVSEModel::getFirmwareVersion(){
    
    return  string("sa").c_str();
}

double EVSEModel::getAmps(){
    double _Amps = 45;
    return _Amps;
}

double EVSEModel::getVoltage(){
    double _Voltage = 2;
    return _Voltage;
}

double EVSEModel::getTotalEnergy(){
    double _TotalEnergy = 12;
    return _TotalEnergy;
}

double EVSEModel::getSessionEnergy(){
    double _SessionEnergy = 2;
    return _SessionEnergy;
}

bool EVSEModel::isVehicleConnected(){
    bool _isVehicleConnected = false;
    return _isVehicleConnected;
}

bool EVSEModel::isCharging(){
    bool _isCharging = false;
    return _isCharging;
}

bool EVSEModel::isActive(){
    bool _isActive = false;
    return _isActive;
}

uint8_t EVSEModel::getEvseState(){
    
}