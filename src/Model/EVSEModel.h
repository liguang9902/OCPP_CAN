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



class EVSEModel:public FiniteStateMachine ,public FirmwareProxy
{
private:
    struct tm sysTimeinfo = { 0 };
    EVSE_Interfacer *emEVSE;

    String chargePointModel;
    int connectorNum;
    String chargePointSerialNumber;
    String chargePointVendor;
    String firmwareVersion;  

    String Cablestatus;
    String CPstatus;
    String lockstutus;

    int L1Voltage; int L1current; int L1Power;
    int L2Voltage; int L2current; int L2Power;
    int L3Voltage; int L3current; int L3Power;

    String idtag;
    int UsingconnecterID;
    String ErrorCode = "NoError";

public:
    EVSEModel(SECC_SPIClass *pCommIF);
    EVSEModel();
    ~EVSEModel();
    
    void loop();

    void Command_Boot();  
    void Command_Heartbeat();
    void Command_MeterValue();
    void Command_Authorize();
    void Command_StopCharing();
    void Command_Error();

    String getchargePointModel();
    int getconnectorNum();
    String getchargePointSerialNumber();
    String getchargePointVendor();
    String getfirmwareVersion();  

    String getCablestatus();
    String getCPstatus();
    String getlockstutus();

    int getL1Voltage(); int getL1current(); int getL1Power();
    int getL2Voltage(); int getL2current(); int getL2Power();
    int getL3Voltage(); int getL3current(); int getL3Power();

    String getidtag();
    int getUsingconnecterID();
    String getErrorCode();
};


#endif
