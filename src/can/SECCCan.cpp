#include "SECCCan.h"
#include "CAN.h"

EVSEModelCan::EVSEModelCan(){
    CAN.setPins(5,4);
    CAN.begin(500E3);
    //CAN.filterExtended();
}

EVSEModelCan::~EVSEModelCan(){

}

void EVSEModelCan::loop(){
   
    
    //char CanSave[8] ={0,};
    String CanSave;
    int packetSize = CAN.parsePacket();
    if (packetSize)
    {
        for (size_t i = 0; i < 8; i++)
            {
            CanSave += (char)CAN.read();
            }
           //Serial.println(CanSave) ;
        CanPacketSave[CAN.packetId()] = CanSave;
        Serial.println(CanPacketSave[CAN.packetId()]);  
    }
    
    CanProtocol_AuthorizeReq(AuthorizeReq);
    CanProtocol_BootNtf(BootNtf);
    CanProtocol_HeartbeatReq(HeartbeatReq);
    CanProtocol_MeterValueNtf(MeterValueNtf);
    CanProtocol_StopChargingNtf(StopChargingNtf);
    CanProtocol_ErrorNtf(ErrorNtf);
    CanProtocol_ChangeAvailabilityRes(ChangeAvailabilityRes);
    CanProtocol_RemoteStartRes(RemoteStartRes);
    CanProtocol_RemoteStopRes(RemoteStopRes);
    CanProtocol_ResetRes(ResetRes);
    CanProtocol_UnlockConnectorRes(UnlockConnectorRes);
   
}

void EVSEModelCan::CanProtocol_AuthorizeReq(Payload_AuthorizeReq& payload){
    //int packetSize = CAN.parsePacket();
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    //Serial.println(CanPacketSave[CanID]);
    //if (packetSize)
    //{
        //Serial.print(CAN.packetId(), HEX);
        char SavedPacket[8];
        if (CanPacketSave[CanID])
        {
            
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            for (size_t i = 0; i < 8; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i];
            }
        }
        
        if (CanPacketSave[CanID - frameID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str());
            for (size_t i = 8; i < 16; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i-8];
            }
        }
        if (CanPacketSave[CanID - frameID*2])
        {
            strcpy(SavedPacket,CanPacketSave[CanID - frameID*2].c_str());
            for (size_t i = 16; i < 20; i++)
            {
            payload.Idtag[i] =SavedPacket[i-16];
            }
            payload.connectorID = (int)SavedPacket[4];
        }
        
    //}
    String IDTag;
    for (size_t i = 0; i < 20; i++)
    {
        IDTag += payload.Idtag[i];
    }
    Authorizeidtag = IDTag;
    AuthorizeCID = payload.connectorID;
    /*
    if (idtag!=NULL)
    {
    Serial.print(" IDTag: ");
    Serial.println(idtag);
    Serial.println(payload.connectorID);
    }
    */
}


void EVSEModelCan::CanProtocol_BootNtf(Payload_BootNtf& payload){
    //int packetSize = CAN.parsePacket();
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
   // if (packetSize)
    //{
        
         if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.CPModel = SavedPacket[0];
            payload.connectorNum = SavedPacket[1];
            for (size_t i = 0; i < 6; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i+2];
            }
        }
        if (CanPacketSave[CanID- frameID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID].c_str());
            for (size_t i = 6; i < 14; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-6];
            }
        }
        if (CanPacketSave[CanID- frameID*2])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*2].c_str());
            for (size_t i = 14; i < 22; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-14];
            }
        }
        if (CanPacketSave[ CanID- frameID*3])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*3].c_str());
            for (size_t i = 22; i < 26; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-22];
            }
            for (size_t i = 0; i < 4; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i+4];
            }
        }
        if (CanPacketSave[ CanID- frameID*4])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*4].c_str());
            for (size_t i = 4; i < 12; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i-4];
            }
        }
        if (CanPacketSave[CanID- frameID*5])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*5].c_str());
            for (size_t i = 12; i < 20; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i-12];
            }
        }
        if (CanPacketSave[CanID- frameID*6])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*6].c_str());
            for (size_t i = 0; i < 8; i++)
            {
            payload.FimwareVersion[i] =(char)SavedPacket[i];
            }
        }
        if (CanPacketSave[CanID- frameID*7])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*7].c_str());
            for (size_t i = 8; i < 16; i++)
            {
            payload.FimwareVersion[i] =(char)SavedPacket[i-8];
            }
        }
   // }
    chargePointModel = protocolCPModel[(CPModel)payload.CPModel]; //用map将数组与string一一对应
    connectorNum = payload.connectorNum;
    String ChargePointSerialNumber;
    String ChargePointVendor;
    String FirmwareVersion;
    for (size_t i = 0; i < 26; i++)
    {
        ChargePointSerialNumber += payload.CPSerialNumber[i];
    }
    for (size_t i = 0; i < 20; i++)
    {
        ChargePointVendor += payload.CPVender[i];
    }
    for (size_t i = 0; i < 16; i++)
    {
        FirmwareVersion += payload.FimwareVersion[i];
    }
    chargePointSerialNumber = ChargePointSerialNumber;
    chargePointVendor = ChargePointVendor;
    firmwareVersion = FirmwareVersion;
    /*
    if (chargePointSerialNumber!=NULL )
    {
    
    Serial.println(chargePointModel);
    Serial.println(payload.connectorNum);
    Serial.println(chargePointSerialNumber);
    Serial.println(chargePointVendor);
    Serial.println(firmwareVersion);
    }
    */
}

void EVSEModelCan::CanProtocol_HeartbeatReq(Payload_HeartbeatReq& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.connectorID = (int)SavedPacket[0];
            payload.statusCB = SavedPacket[1];
            payload.statusCP = SavedPacket[2];
            payload.statusLock = SavedPacket[3];
        }
    HeartbeatCID = payload.connectorID;
    Cablestatus = protocolCableStatus[(CableStatus)payload.statusCB];
    CPstatus = protocolCPStatus[(CPStatus)payload.statusCP];
    lockstutus = protocolLockStatus[(LockStatus)payload.statusLock];
    /*if (lockstutus!=NULL )
    {
    
    Serial.println(UsingconnecterID);
    Serial.println(Cablestatus);
    Serial.println(CPstatus);
    Serial.println(lockstutus);
    }*/
}

void EVSEModelCan::CanProtocol_MeterValueNtf(Payload_MeterValueNtf& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.currentTime.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.currentTime.tm_mon = (int)SavedPacket[2];
            payload.currentTime.tm_mday = (int)SavedPacket[3];
            payload.currentTime.tm_hour = (int)SavedPacket[4];
            payload.currentTime.tm_min = (int)SavedPacket[5];
            payload.currentTime.tm_sec = (int)SavedPacket[6];
        }
    if (CanPacketSave[CanID- frameID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID].c_str());
            payload.connectorID = (int)SavedPacket[0];
            payload.L1Voltage = (float)SavedPacket[1]*100+(float)SavedPacket[2]+(float)SavedPacket[3]/100;
            payload.L1Current = (float)SavedPacket[4]*100+(float)SavedPacket[5]+(float)SavedPacket[6]/100;
        }
    if (CanPacketSave[CanID- frameID*2])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*2].c_str());
            payload.L1Power = (float)SavedPacket[0]*100+(float)SavedPacket[1]+(float)SavedPacket[2]/100;
            payload.L2Voltage = (float)SavedPacket[3]*100+(float)SavedPacket[4]+(float)SavedPacket[5]/100;
        }    
    if (CanPacketSave[CanID- frameID*3])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*3].c_str());
            payload.L2Current = (float)SavedPacket[0]*100+(float)SavedPacket[1]+(float)SavedPacket[2]/100;
            payload.L2Power = (float)SavedPacket[3]*100+(float)SavedPacket[4]+(float)SavedPacket[5]/100;
        }        
    if (CanPacketSave[CanID- frameID*4])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*4].c_str());
            payload.L3Voltage = (float)SavedPacket[0]*100+(float)SavedPacket[1]+(float)SavedPacket[2]/100;
            payload.L3Current = (float)SavedPacket[3]*100+(float)SavedPacket[4]+(float)SavedPacket[5]/100;
        }    
    if (CanPacketSave[CanID- frameID*5])
        {
            strcpy(SavedPacket,CanPacketSave[CanID- frameID*5].c_str());
            payload.L3Power = (float)SavedPacket[0]*100+(float)SavedPacket[1]+(float)SavedPacket[2]/100;
        }    
    //char strftime_buf[64]={0,};
    //strftime(strftime_buf, 64, "%FT%T", &payload.currentTime);
    MeterValueTimestamp = payload.currentTime; //还有问题
    MeterValueCID = payload.connectorID;
    L1Voltage = payload.L1Voltage;L1Current = payload.L1Current;L1Power = payload.L1Power;
    L2Voltage = payload.L2Voltage;L2Current = payload.L2Current;L2Power = payload.L2Power;
    L3Voltage = payload.L3Voltage;L3Current = payload.L3Current;L3Power = payload.L3Power;
    /*
    Serial.println(currentTime);
    Serial.println(UsingconnecterID);
    Serial.println(L1Voltage);
    Serial.println(L1Current);
    Serial.println(L1Power); 
    Serial.println(L2Voltage);
    Serial.println(L2Current);
    Serial.println(L2Power);
    Serial.println(L3Voltage);
    Serial.println(L3Current);
    Serial.println(L3Power);   */
}

void EVSEModelCan::CanProtocol_StopChargingNtf(Payload_StopChargingNtf& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
           for (size_t i = 0; i < 8; i++)
            {
            payload.Idtag[i] =SavedPacket[i];
            } 
        }
    if (CanPacketSave[CanID - frameID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str());
            for (size_t i = 8; i < 16; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i-8];
            }
        }
    if (CanPacketSave[CanID - frameID*2])
        {
            strcpy(SavedPacket,CanPacketSave[CanID - frameID*2].c_str());
            for (size_t i = 16; i < 20; i++)
            {
            payload.Idtag[i] =SavedPacket[i-16];
            }
            payload.MeterStop = (float)SavedPacket[4]*100+(float)SavedPacket[5]+(float)SavedPacket[6]/100;
            payload.StopReason = SavedPacket[7];
        }
    if (CanPacketSave[CanID-frameID*3])
        {
            strcpy(SavedPacket,CanPacketSave[CanID-frameID*3].c_str());
            payload.Timestamp.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.Timestamp.tm_mon = (int)SavedPacket[2];
            payload.Timestamp.tm_mday = (int)SavedPacket[3];
            payload.Timestamp.tm_hour = (int)SavedPacket[4];
            payload.Timestamp.tm_min = (int)SavedPacket[5];
            payload.Timestamp.tm_sec = (int)SavedPacket[6];
        }
    if (CanPacketSave[CanID-frameID*4])
        {
            strcpy(SavedPacket,CanPacketSave[CanID-frameID*4].c_str());
            payload.connectorID = (int)SavedPacket[0];
        }
        String IDTag;
    for (size_t i = 0; i < 20; i++)
        {
            IDTag += payload.Idtag[i];
        }
    StopChargingidtag = IDTag;
    StopChargingCID = payload.connectorID;
    MeterStop = payload.MeterStop;
    StopChargingTimestamp = payload.Timestamp;
    StopReason = protocolStopCharingReason[(StopCharingReason)payload.StopReason];

    /*
    char strftime_buf[64]={0,};
    strftime(strftime_buf, 64, "%FT%T", &payload.Timestamp);
    String Times;
    Times = strftime_buf;
    Serial.println(Times);
    Serial.println(StopChargingidtag);
    Serial.println(StopChargingCID);
    Serial.println(MeterStop);
    Serial.println(StopReason);*/
}

void EVSEModelCan::CanProtocol_ErrorNtf(Payload_ErrorNtf& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.Timestamp.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.Timestamp.tm_mon = (int)SavedPacket[2];
            payload.Timestamp.tm_mday = (int)SavedPacket[3];
            payload.Timestamp.tm_hour = (int)SavedPacket[4];
            payload.Timestamp.tm_min = (int)SavedPacket[5];
            payload.Timestamp.tm_sec = (int)SavedPacket[6];
        }
    if (CanPacketSave[CanID - frameID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str());
            payload.connectorID = (int)SavedPacket[0];
            payload.ReasonError = SavedPacket[1];
        }    
    ErrorTimestamp = payload.Timestamp;
    ErrorCID = payload.connectorID;
    ErrorCode = protocolErrorReason[(ErrorReason)payload.ReasonError];
    /*
    char strftime_buf[64]={0,};
    strftime(strftime_buf, 64, "%FT%T", &payload.Timestamp);
    String Times;
    Times = strftime_buf;
    Serial.println(Times);
    Serial.println(ErrorCID);
    Serial.println(ErrorCode);*/
}

void EVSEModelCan::CanProtocol_ChangeAvailabilityRes(Payload_ChangeAvailabilityRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.CAResstatus = SavedPacket[0];
        }
    ChangeAvailabilityStatus = protocolChangeAvailabilityResStatus[(ChangeAvailabilityResStatus)payload.CAResstatus];
    //Serial.println(ChangeAvailabilityStatus);
}

void EVSEModelCan::CanProtocol_RemoteStartRes(Payload_RemoteStartRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.status = SavedPacket[0];
        }
    RemoteStartStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(RemoteStartStatus);
}

void EVSEModelCan::CanProtocol_RemoteStopRes(Payload_RemoteStopRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.status = SavedPacket[0];
        }
    RemoteStopStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(RemoteStopStatus);
}

void EVSEModelCan::CanProtocol_ResetRes(Payload_ResetRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.status = SavedPacket[0];
        }
    ResetStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(ResetStatus);
}

void EVSEModelCan::CanProtocol_UnlockConnectorRes(Payload_UnlockConnectorRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if (CanPacketSave[CanID])
        {
            strcpy(SavedPacket,CanPacketSave[CanID].c_str());
            payload.StatusUnlockConnector = SavedPacket[0];
        }
    UnlockconnectorStatus = protocolUnlockConnectorStatus[(UnlockConnectorStatus)payload.StatusUnlockConnector];
    //Serial.println(UnlockconnectorStatus);
}