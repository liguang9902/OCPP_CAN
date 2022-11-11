#ifndef SECCCAN_H
#define SECCCAN_H


#include <map>
#include <functional>
#include <time.h>
#include "NewProtocol.h"

#include <map>

class EVSEModelCan
{
private:

    String chargePointModel;
    int connectorNum;
    String chargePointSerialNumber;
    String chargePointVendor;
    String firmwareVersion;  

    uint8_t Cablestatus;
    uint8_t CPstatus;
    uint8_t lockstutus;

    float L1Voltage; float L1Current; float L1Power;
    float L2Voltage; float L2Current; float L2Power;
    float L3Voltage; float L3Current; float L3Power;

    float TotalElectricity;

    float MeterStop;
    //tm currentTime;
    tm MeterValueTimestamp;tm StopChargingTimestamp;tm ErrorTimestamp;
    String Authorizeidtag;String StopChargingidtag;
    int AuthorizeCID;int HeartbeatCID;int MeterValueCID;int StopChargingCID;int ErrorCID;
    String ErrorCode = "NoError";
    String StopReason;

    String ChangeAvailabilityStatus = "Invalid";
    String RemoteStartStatus = "Invalid";
    String RemoteStopStatus = "Invalid";
    String ResetStatus = "Invalid";
    String UnlockconnectorStatus = "Invalid";

    Payload_AuthorizeReq  AuthorizeReq;
    Payload_BootNtf BootNtf;
    Payload_HeartbeatReq HeartbeatReq;
    Payload_MeterValueNtf MeterValueNtf;
    Payload_StopChargingNtf StopChargingNtf;
    Payload_ErrorNtf ErrorNtf;
    Payload_ChangeAvailabilityCnf ChangeAvailabilityCnf;
    Payload_RemoteStartRes RemoteStartRes;
    Payload_RemoteStopRes RemoteStopRes;
    Payload_ResetRes ResetRes;
    Payload_UnlockConnectorRes UnlockConnectorRes;
    Payload_ChangeAvailabilityNtf ChangeAvailabilityNtf;

    
    //template<typename T>
    void CanProtocol_AuthorizeReq(Payload_AuthorizeReq& payload );  
    void CanProtocol_BootNtf(Payload_BootNtf& payload);
    void CanProtocol_HeartbeatReq(Payload_HeartbeatReq& payload);
    void CanProtocol_MeterValueNtf(Payload_MeterValueNtf& payload);
    void CanProtocol_StopChargingNtf(Payload_StopChargingNtf& payload);
    void CanProtocol_ErrorNtf(Payload_ErrorNtf& payload);
    void CanProtocol_ChangeAvailabilityCnf(Payload_ChangeAvailabilityCnf& payload);
    void CanProtocol_RemoteStartRes(Payload_RemoteStartRes& payload);
    void CanProtocol_RemoteStopRes(Payload_RemoteStopRes& payload);
    void CanProtocol_ResetRes(Payload_ResetRes& payload);
    void CanProtocol_UnlockConnectorRes(Payload_UnlockConnectorRes& payload);
    //void CanProtocol_ChangeAvailabilityNtf(Payload_ChangeAvailabilityNtf& payload);
public:
    //EVSEModel(SECC_SPIClass *pCommIF);
    EVSEModelCan();
    ~EVSEModelCan();
    
    void loop();

    bool authorizeFlag = false;
    bool BootFlag = false;
    std::map<uint32_t,String> CanPacketSave;
    String getchargePointModel();
    int getconnectorNum();
    String getchargePointSerialNumber();
    String getchargePointVendor();
    String getfirmwareVersion();  

    uint8_t getCablestatus();
    uint8_t getCPstatus();
    uint8_t getlockstutus();

    float getL1Voltage(); float getL1current(); float getL1Power();
    float getL2Voltage(); float getL2current(); float getL2Power();
    float getL3Voltage(); float getL3current(); float getL3Power();

    String getidtag();
    int getUsingconnecterID();
    String getErrorCode();
    float getMeterStop();
    float getTotalElectricity();

    tm getMeterValueTimestamp();tm getStopChargingTimestamp();tm getErrorTimestamp();
    String getAuthorizeidtag();String getStopChargingidtag();
    int getAuthorizeCID();int getHeartbeatCID();int getMeterValueCID();int getStopChargingCID();int getErrorCID();
    
    String getStopReason();

    String getChangeAvailabilityStatus();
    String getRemoteStartStatus();
    String getRemoteStopStatus();
    String getResetStatus();
    String getUnlockconnectorStatus();
};

extern EVSEModelCan  Canmodel;
#endif