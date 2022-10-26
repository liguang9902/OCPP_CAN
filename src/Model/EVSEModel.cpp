
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <FS.h>
#include <core/NewOCPP.h>
#include <ArduinoOcpp/Tasks/SmartCharging/SmartChargingService.h>

#include "Model/EVSEModel.h"
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

EVSEModel::EVSEModel(SECC_SPIClass *pCommIF)
{   
    
    emEVSE = new EVSE_Interfacer(pCommIF);
}

EVSEModel::EVSEModel(){
    
}

EVSEModel::~EVSEModel()
{
}

