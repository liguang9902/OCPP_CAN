#include "NewProtocol.h"
#include "CAN.h"
#include "SECCCan.h"

using namespace ArduinoOcpp ;

#define cID 99
const uint32_t newProtocolCommand[ProtocolCommand_MAX][1]=
{
    0x00232A1,
    0x010A132,
    0x12732A1,
    0x131A132,
    0x20032A1,
    0x211A132,
    0x32432A1,
    0x330A132,
    0x42432A1,
    0x430A132,
    0x52132A1,
    0x530A132,
    0x620A132,
    0x63032A1,
    0x702A132,
    0x71032A1,
    0x800A132,
    0x81032A1,
    0x900A132,
    0x91032A1,
    0xA00A132,
    0xA1032A1,
    0xB00A132,
    0xB1032A1,
    0xC07A132,
    0xC1032A1,
};

std::map<CPModel , String> protocolCPModel{
    {AC,"AC"},
    {DC,"DC"}
};

std::map<CableStatus , String> protocolCableStatus{
    {plugged,"plugged"},
    {unplugged,"unplugged"}
};

std::map<CPStatus , String> protocolCPStatus{
    {CP_Status_UNKNOWN,"UNKNOWN"},	
	{CP_Status_A,"A"},		
	{CP_Status_B,"B"},		
	{CP_Status_C,"C"},		
	{CP_Status_D,"D"},				
	{CP_Status_E,"E"},
    {CP_Status_F,"F"},
    {CP_Status_INVALID,"INVALID"},
};

std::map<LockStatus , String> protocolLockStatus{
    {LockStatus_UNKNOWN,"UNKNOWN"},
    {locked,"locked"},
    {unlocked,"unlocked"},
    {LockStatus_INVALID,"INVALID"},
};

std::map<StopCharingReason , String> protocolStopCharingReason{
    {Local,"Local"},
    {DeAuthorized,"DeAuthorized"},
    {EmergencyStop,"EmergencyStop"},
    {EVDisconnected,"EVDisconnected"},
    {HardReset,"HardReset"},
    {Reboot,"Reboot"},
    {Remote,"Remote"},
    {PoweLoss,"PoweLoss"},
    {SoftReset,"SoftReset"}, 		
    {UnlockCommand,"UnlockCommand"}
};

std::map<ErrorReason , String> protocolErrorReason{
    {ConnectorLockFailure,"ConnectorLockFailure"},
    {EVCommunicationError,"EVCommunicationError"},
    {GroundFailure,"GroundFailure"},
    {HighTemperature,"HighTemperature"},	
    {InternalError,"InternalError"}, 		
    {LocalListConflict,"LocalListConflict"}, 
    {OverCurrentFailure,"OverCurrentFailure"},
    {OverVoltage,"OverVoltage"},
    {PowerMeterFailure,"PowerMeterFailure"}, 
    {PowerSwitchFailure,"PowerSwitchFailure"},
    {ReaderFailure,"ReaderFailure"},
    {UnderVoltage,"UnderVoltage"},
    {WeakSignal,"WeakSignal"},
    {NoError,"NoError"}
};

std::map<ChangeAvailabilityCnfStatus , String> protocolChangeAvailabilityCnfStatus{
    {CA_Accepted,"Accepted"},
    {CA_Rejectedcked,"Rejected"}
    //{CAS_Scheduled,"Scheduled"}
};

std::map<CommonStatus , String> protocolCommonStatus{
    {Accepted,"Accepted"},
    {Rejected,"Rejected"}
};

std::map<UnlockConnectorStatus , String> protocolUnlockConnectorStatus{
    {Unlocked,"Unlocked"},
    {UnlockFailed,"UnlockFailed"},
    {NotSupported,"NotSupported"}
};

std::map<AuthorizeStatus , String> protocolAuthorizeStatus{
    {AS_Accepted,"Accepted"},       
    {AS_Blocked,"Blocked"},        
    {AS_ConcurrentTx,"ConcurrentTx"},  
    {AS_Expired,"Expired"},        
    {AS_Invalid,"Invalid"} 
};

std::map<EVSEStatus , String> protocolEVSEStatus{
    {Available,"Available"},
    {Preparing,"Preparing"},
    {Charging,"Charging"},
    {SuspendedEV,"SuspendedEV"},
    {SuspendedEVSE,"SuspendedEVSE"},
    {Finishing,"Finishing"}, 	
    {Reserved,"Reserved"}, 		
    {Unavailable,"Unavailable"},	
    {Faulted,"Faulted"}
};

std::map<ResetType , String> protocolResetType{
    {Hard,"Hard"},
    {Soft,"Soft"}
};
String ID = "12345678901234567890";
tm TestTime ;

#define NumConnectors 2
uint8_t SavedAvailablityStatus[NumConnectors] = {0,};
/*
template<>
void Canpacket_ProtocolSend<Payload_AuthorizeReq>(Payload_AuthorizeReq& payload){
    strcpy(payload.Idtag,ID.c_str());
    payload.connectorID = cID;

    //packet.CanID = *newProtocolCommand[ProtocolCommand_Authorizereq];
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    //CAN.write((uint8_t*)ID.c_str(),8);
    for (size_t i = 0; i < 8; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    for (size_t i = 8; i < 16; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    for (size_t i = 16; i < 20; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.write(payload.connectorID);
    CAN.endPacket();
}
*/



template<>
void Canpacket_ProtocolSend<Payload_AuthorizeRes>(Payload_AuthorizeRes& payload,JsonObject confMsg){
    //payload.Authorizestatus = AS_Blocked;

    payload.Authorizestatus = protocolAuthorizeStatus.find(confMsg["idTagInfo"]["status"])->first;
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.Authorizestatus);
    //CAN.write(AS);
    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_BootCnf>(Payload_BootCnf& payload,JsonObject confMsg){
    /*TestTime.tm_year = 2022;
    TestTime.tm_mon  = 8 ;
    TestTime.tm_mday = 29;
    TestTime.tm_hour = 10;
    TestTime.tm_min  = 50;
    TestTime.tm_sec  = 32;*/
    //const char *currentTime = confMsg["currentTime"] | "Invalid";
    //validateOcppTime(currentTime , payload.currentTime);
    
    esp_get_systime(payload.currentTime);

    //payload.currentTime = TestTime;
    payload.HeartBeatInterval = 10;
    payload.MeterValueInterval = 10;
    //payload.BootStatus = Accepted;
    payload.BootStatus = protocolCommonStatus.find(confMsg["status"])->first;//还未测试

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.currentTime.tm_year/100);
    CAN.write(payload.currentTime.tm_year%100);
    CAN.write(payload.currentTime.tm_mon);
    CAN.write(payload.currentTime.tm_mday);
    CAN.write(payload.currentTime.tm_hour);
    CAN.write(payload.currentTime.tm_min);
    CAN.write(payload.currentTime.tm_sec);
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.HeartBeatInterval);
    CAN.write(payload.MeterValueInterval);
    CAN.write(payload.BootStatus);
    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_HeartbeatRes>(Payload_HeartbeatRes& payload){
   /* TestTime.tm_year = 2022;
    TestTime.tm_mon  = 8 ;
    TestTime.tm_mday = 29;
    TestTime.tm_hour = 10;
    TestTime.tm_min  = 50;
    TestTime.tm_sec  = 32;*/
    //const char *currentTime = confMsg["currentTime"] | "Invalid";
    //validateOcppTime(currentTime , payload.currentTime);
    //payload.currentTime = TestTime;
    esp_get_systime(payload.currentTime);
    ArduinoOcpp::ChargePointStatusService *CPSService = getChargePointStatusService();
    auto connectorStatus= CPSService->getConnector(Canmodel.getHeartbeatCID());
    payload.statusEVSE = (EVSEStatus)connectorStatus->inferenceStatus(); //?

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.currentTime.tm_year/100);
    CAN.write(payload.currentTime.tm_year%100);
    CAN.write(payload.currentTime.tm_mon);
    CAN.write(payload.currentTime.tm_mday);
    CAN.write(payload.currentTime.tm_hour);
    CAN.write(payload.currentTime.tm_min);
    CAN.write(payload.currentTime.tm_sec);
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.statusEVSE);
    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_MeterValueCnf>(Payload_MeterValueCnf& payload){

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);

    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_StopChargingCnf>(Payload_StopChargingCnf& payload){

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);

    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_ErrorCnf>(Payload_ErrorCnf& payload){

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);

    CAN.endPacket();
}

void initialiSavedAvailablityStatus(){
    ArduinoOcpp::ChargePointStatusService *CPSService = getChargePointStatusService();
   
    for (int i = 0; i < CPSService->getNumConnectors(); i++) {
    SavedAvailablityStatus[i] = CPSService->getConnector(i)->getAvailability();
    }
}

template<>
void Canpacket_ProtocolSend<Payload_ChangeAvailabilityNtf>(Payload_ChangeAvailabilityNtf& payload){
    ArduinoOcpp::ChargePointStatusService *CPSService = getChargePointStatusService();

    for (int i = 0; i < CPSService->getNumConnectors(); i++) {
        //if(CPSService->getConnector(i)->getAvailability()!=SavedAvailablityStatus[i]){
        payload.connectorID = i;
        payload.CAReqstatus = (ChangeAvailabilityNtfStatus)CPSService->getConnector(i)->getAvailability();//

        uint32_t CanID = *newProtocolCommand[payload.CmdID];
        CAN.beginExtendedPacket(CanID, 8);
        CAN.write(payload.connectorID);
        CAN.write(payload.CAReqstatus);
        CAN.endPacket();
        //}
    }
}  

template<>
void Canpacket_ProtocolSend<Payload_RemoteStartReq>(Payload_RemoteStartReq& payload,JsonObject confMsg){
    //payload.connectorID = cID;
    //strcpy(payload.Idtag,ID.c_str());
    payload.connectorID = confMsg["connectorId"] | -1;
    strcpy(payload.Idtag,confMsg["idTag"] | "INVALID");

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    for (size_t i = 0; i < 8; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    for (size_t i = 8; i < 16; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.endPacket();

    CanID = CanID-frameID;
    CAN.beginExtendedPacket(CanID, 8);
    for (size_t i = 16; i < 20; i++)
    {
        CAN.write(payload.Idtag[i]);
    }
    CAN.write(payload.connectorID);
    CAN.endPacket();
    
}

template<>
void Canpacket_ProtocolSend<Payload_RemoteStopReq>(Payload_RemoteStopReq& payload,JsonObject confMsg){
    //payload.connectorID = cID;
    payload.connectorID = confMsg["connectorId"] | -1;

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.connectorID);
    CAN.endPacket();
}

template<>
void Canpacket_ProtocolSend<Payload_ResetReq>(Payload_ResetReq& payload,JsonObject confMsg){
    //payload.type = Hard;
    payload.type = confMsg["type"];

    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.type);
    CAN.endPacket();
}

#define ID_OF_CONNECTOR 1
template<>
void Canpacket_ProtocolSend<Payload_UnlockConnectorReq>(Payload_UnlockConnectorReq& payload){
    //payload.connectorID = cID;
    //payload.connectorID = confMsg["connectorId"] | -1;
    payload.connectorID = ID_OF_CONNECTOR;
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    CAN.beginExtendedPacket(CanID, 8);
    CAN.write(payload.connectorID);
    CAN.endPacket();
}
/*
template<>
void Canunpacket_ProtocolRes<Payload_AuthorizeReq>(Payload_AuthorizeReq& payload){
    int packetSize = CAN.parsePacket();
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    if (packetSize)
    {
        Serial.print(CAN.packetId(), HEX);
        if (CAN.packetId() == CanID)
        {
            for (size_t i = 0; i < 8; i++)
            {
            payload.Idtag[i] =(char)CAN.read();
            }
        }
        
        if (CAN.packetId() == CanID - frameID)
        {
            for (size_t i = 8; i < 16; i++)
            {
            payload.Idtag[i] =(char)CAN.read();
            }
        }
        if (CAN.packetId() == CanID - frameID*2)
        {
            for (size_t i = 16; i < 20; i++)
            {
            payload.Idtag[i] =(char)CAN.read();
            }
            payload.connectorID = CAN.read();
        }
        
    }
    String IDTag;
    for (size_t i = 0; i < 20; i++)
    {
        IDTag += payload.Idtag[i];
    }

    if (IDTag!=NULL)
    {
    Serial.print(" IDTag: ");
    Serial.println(IDTag);
    Serial.println(payload.connectorID);
    }
    
    
}
*/