#include "SECCCan.h"
#include "CAN.h"

EVSEModelCan::EVSEModelCan(){
    CAN.setPins(5,4);
    CAN.begin(500E3);
    //CAN.filterExtended();
   //initialiSavedAvailablityStatus();
}

EVSEModelCan::~EVSEModelCan(){

}

void EVSEModelCan::loop(){
   
    
    //char CanSave[8] ={0,};
    String CanSave;
    int packetSize = CAN.parsePacket();
    if (packetSize)
    {
        while (CAN.available()){
        for (size_t i = 0; i < 8; i++)
            {
            CanSave += (char)CAN.read();
            }
           //Serial.println(CanSave) ;
        CanPacketSave[CAN.packetId()] = CanSave;
       // Serial.println(CanPacketSave[CAN.packetId()]);  
        }
    }
    
    //CanProtocol_ChangeAvailabilityNtf(ChangeAvailabilityNtf);// 发送状态更改通知
    CanProtocol_AuthorizeReq(AuthorizeReq);
    CanProtocol_BootNtf(BootNtf);
    CanProtocol_HeartbeatReq(HeartbeatReq);
    CanProtocol_MeterValueNtf(MeterValueNtf);
    CanProtocol_StopChargingNtf(StopChargingNtf);
    CanProtocol_ErrorNtf(ErrorNtf);
    CanProtocol_ChangeAvailabilityCnf(ChangeAvailabilityCnf);
    CanProtocol_RemoteStartRes(RemoteStartRes);
    CanProtocol_RemoteStopRes(RemoteStopRes);
    CanProtocol_ResetRes(ResetRes);
    CanProtocol_UnlockConnectorRes(UnlockConnectorRes);
    
    //Canpacket_ProtocolSend(ChangeAvailabilityNtf);
}

void EVSEModelCan::CanProtocol_AuthorizeReq(Payload_AuthorizeReq& payload){
    //int packetSize = CAN.parsePacket();
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    //Serial.println(CanPacketSave[CanID]);
    //if (packetSize)
    //{
        //Serial.print(CAN.packetId(), HEX);
        char SavedPacket[8];
        if(CanPacketSave.count(CanID))
        {
            
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.connectorID = (int)SavedPacket[0];
            payload.Length = (int)SavedPacket[1];
            for (size_t i = 0; i < 4; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i+4];
            }
            
            CanPacketSave.erase(CanID);
        }
        
        if(CanPacketSave.count(CanID - frameID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str(),8);
            for (size_t i = 4; i < 12; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i-4];
            }
            CanPacketSave.erase(CanID - frameID);
        }
        if(CanPacketSave.count(CanID - frameID*2))
        {
            memcpy(SavedPacket,CanPacketSave[CanID - frameID*2].c_str(),8);
            for (size_t i = 12; i < 20; i++)
            {
            payload.Idtag[i] =SavedPacket[i-12];
            }
            authorizeFlag = true;
            CanPacketSave.erase(CanID - frameID*2);
        }
        
    //}
    String IDTag;
    IDtaglength = payload.Length;
    /*for (size_t i = 0; i < IDtaglength; i++)
    {
        IDTag += payload.Idtag[i];
    }*/
    for (size_t i = 0; i < IDtaglength; i++)
   {
        IDTag += HexToAscii(payload.Idtag[i]);
   }
   
    
    Authorizeidtag = IDTag;
    AuthorizeCID = payload.connectorID;
    
    /*
    Serial.println(IDtaglength);
    Serial.println(Authorizeidtag);
    Serial.println(payload.Idtag);
    */
    
}


void EVSEModelCan::CanProtocol_BootNtf(Payload_BootNtf& payload){
    //int packetSize = CAN.parsePacket();
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
   // if (packetSize)
    //{
        
         if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.CPModel = SavedPacket[0];
            payload.connectorNum = (int)SavedPacket[1];
            for (size_t i = 0; i < 6; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i+2];
            }
             CanPacketSave.erase(CanID);
        }
        if(CanPacketSave.count(CanID- frameID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID].c_str(),8);
            for (size_t i = 6; i < 14; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-6];
            }
            CanPacketSave.erase(CanID - frameID);
        }
        if(CanPacketSave.count(CanID- frameID*2))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*2].c_str(),8);
            for (size_t i = 14; i < 22; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-14];
            }
            CanPacketSave.erase(CanID - frameID*2);
        }
        if(CanPacketSave.count( CanID- frameID*3))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*3].c_str(),8);
            for (size_t i = 22; i < 26; i++)
            {
            payload.CPSerialNumber[i] =(char)SavedPacket[i-22];
            }
            for (size_t i = 0; i < 4; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i+4];
            }
            CanPacketSave.erase(CanID - frameID*3);
        }
        if(CanPacketSave.count( CanID- frameID*4))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*4].c_str(),8);
            for (size_t i = 4; i < 12; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i-4];
            }
            CanPacketSave.erase(CanID - frameID*4);
        }
        if(CanPacketSave.count(CanID- frameID*5))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*5].c_str(),8);
            for (size_t i = 12; i < 20; i++)
            {
            payload.CPVender[i] =(char)SavedPacket[i-12];
            }
            CanPacketSave.erase(CanID - frameID*5);
        }
        if(CanPacketSave.count(CanID- frameID*6))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*6].c_str(),8);
            for (size_t i = 0; i < 8; i++)
            {
            payload.FimwareVersion[i] =(char)SavedPacket[i];
            }
            CanPacketSave.erase(CanID - frameID*6);
        }
        if(CanPacketSave.count(CanID- frameID*7))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*7].c_str(),8);
            for (size_t i = 8; i < 16; i++)
            {
            payload.FimwareVersion[i] =(char)SavedPacket[i-8];
            }
            this->BootFlag = true;
            CanPacketSave.erase(CanID - frameID*7);
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
    Serial.println(chargePointModel);
    Serial.println(connectorNum);
    Serial.println(chargePointSerialNumber);
    Serial.println(chargePointVendor);
    Serial.println(firmwareVersion);
    */
    
}

void EVSEModelCan::CanProtocol_HeartbeatReq(Payload_HeartbeatReq& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.connectorID = (int)SavedPacket[0];
            payload.statusCB = SavedPacket[1];
            payload.statusCP = SavedPacket[2];
            payload.statusLock = SavedPacket[3];
            Payload_HeartbeatRes HeartbeatRes;
            Canpacket_ProtocolSend(HeartbeatRes);
            CanPacketSave.erase(CanID);
        }
    HeartbeatCID = payload.connectorID;
    //Cablestatus = protocolCableStatus[(CableStatus)payload.statusCB];
    //CPstatus = protocolCPStatus[(CPStatus)payload.statusCP];
    //lockstutus = protocolLockStatus[(LockStatus)payload.statusLock];
    Cablestatus = (CableStatus)payload.statusCB;
    CPstatus = payload.statusCP;
    lockstutus = (LockStatus)payload.statusLock;
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
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.currentTime.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.currentTime.tm_mon = (int)SavedPacket[2];
            payload.currentTime.tm_mday = (int)SavedPacket[3];
            payload.currentTime.tm_hour = (int)SavedPacket[4];
            payload.currentTime.tm_min = (int)SavedPacket[5];
            payload.currentTime.tm_sec = (int)SavedPacket[6];
            
            CanPacketSave.erase(CanID);
        }
    if(CanPacketSave.count(CanID- frameID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID].c_str(),8);
            //auto k = CanPacketSave[CanID- frameID].c_str();
            payload.connectorID = (int)SavedPacket[0];
            payload.TotalElectricity = ((float)((uint32_t)((SavedPacket[1]<<24) | (SavedPacket[2]<<16) | (SavedPacket[3]<<8) | SavedPacket[4])))/100;
            CanPacketSave.erase(CanID - frameID);
        }
    if(CanPacketSave.count(CanID- frameID*2))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*2].c_str(),8);
            payload.L1Voltage = ((float)((uint16_t)((SavedPacket[0]<<8) | SavedPacket[1])))/100;
            payload.L1Current = ((float)((uint16_t)((SavedPacket[2]<<8) | SavedPacket[3])))/100;
            payload.L1Power = ((float)((uint32_t)((SavedPacket[4]<<24) | (SavedPacket[5]<<16) | (SavedPacket[6]<<8) | SavedPacket[7])))/10;
            CanPacketSave.erase(CanID - frameID*2);
        }    
    if(CanPacketSave.count(CanID- frameID*3))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*3].c_str(),8);
            payload.L2Voltage =((float)((uint16_t)((SavedPacket[0]<<8) | SavedPacket[1])))/100;
            payload.L2Current =((float)((uint16_t)((SavedPacket[2]<<8) | SavedPacket[3])))/100;
            payload.L2Power = ((float)((uint32_t)((SavedPacket[4]<<24) | (SavedPacket[5]<<16) | (SavedPacket[6]<<8) | SavedPacket[7])))/10;
            CanPacketSave.erase(CanID - frameID*3);
        }        
    if(CanPacketSave.count(CanID- frameID*4))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*4].c_str(),8);
            payload.L3Voltage = ((float)((uint16_t)((SavedPacket[0]<<8) | SavedPacket[1])))/100;
            payload.L3Current = ((float)((uint16_t)((SavedPacket[2]<<8) | SavedPacket[3])))/100;
            payload.L3Power =  ((float)((uint32_t)((SavedPacket[4]<<24) | (SavedPacket[5]<<16) | (SavedPacket[6]<<8) | SavedPacket[7])))/10;
            Payload_MeterValueCnf MeterValueCnf;
            Canpacket_ProtocolSend(MeterValueCnf);
            CanPacketSave.erase(CanID - frameID*4);
        }    
    /*if(CanPacketSave.count(CanID- frameID*5))
        {
            memcpy(SavedPacket,CanPacketSave[CanID- frameID*5].c_str());
            payload.L3Power = (float)SavedPacket[0]*100+(float)SavedPacket[1]+(float)SavedPacket[2]/100;
            payload.TotalElectricity = (float)SavedPacket[3]*100+(float)SavedPacket[4]+(float)SavedPacket[5]/100;
            Payload_MeterValueCnf MeterValueCnf;
            Canpacket_ProtocolSend(MeterValueCnf);
            CanPacketSave.erase(CanID- frameID*5);
        }    */
    //char strftime_buf[64]={0,};
    //strftime(strftime_buf, 64, "%FT%T", &payload.currentTime);
    MeterValueTimestamp = payload.currentTime; //还有问题
    MeterValueCID = payload.connectorID;
    L1Voltage = payload.L1Voltage;L1Current = payload.L1Current;L1Power = payload.L1Power;
    L2Voltage = payload.L2Voltage;L2Current = payload.L2Current;L2Power = payload.L2Power;
    L3Voltage = payload.L3Voltage;L3Current = payload.L3Current;L3Power = payload.L3Power;
    TotalElectricity = payload.TotalElectricity;
    /*Serial.println(payload.TotalElectricity);
    //Serial.println(currentTime);
    Serial.println(MeterValueCID);
    Serial.println(L1Voltage);
    Serial.println(L1Current);
    Serial.println(L1Power); 
    Serial.println(L2Voltage);
    Serial.println(L2Current);
    Serial.println(L2Power);
    Serial.println(L3Voltage);
    Serial.println(L3Current);
    Serial.println(L3Power);*/   
}

void EVSEModelCan::CanProtocol_StopChargingNtf(Payload_StopChargingNtf& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
           for (size_t i = 0; i < 8; i++)
            {
            payload.Idtag[i] =SavedPacket[i];
            } 
            Payload_StopChargingCnf StopChargingCnf;
            Canpacket_ProtocolSend(StopChargingCnf);
            CanPacketSave.erase(CanID);
        }
    if(CanPacketSave.count(CanID - frameID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str(),8);
            for (size_t i = 8; i < 16; i++)
            {
            //payload.Idtag[i] =(char)CAN.read();
            payload.Idtag[i] =SavedPacket[i-8];
            }
            CanPacketSave.erase(CanID - frameID);
        }
    if(CanPacketSave.count(CanID - frameID*2))
        {
            memcpy(SavedPacket,CanPacketSave[CanID - frameID*2].c_str(),8);
            for (size_t i = 16; i < 20; i++)
            {
            payload.Idtag[i] =SavedPacket[i-16];
            }
            payload.MeterStop = ((float)((uint32_t)((SavedPacket[4]<<24) | (SavedPacket[5]<<16) | (SavedPacket[6]<<8) | SavedPacket[7])))/100;
            
            CanPacketSave.erase(CanID - frameID*2);
        }
    if(CanPacketSave.count(CanID - frameID*3))
        {
            memcpy(SavedPacket,CanPacketSave[CanID-frameID*3].c_str(),8);
            payload.Timestamp.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.Timestamp.tm_mon = (int)SavedPacket[2];
            payload.Timestamp.tm_mday = (int)SavedPacket[3];
            payload.Timestamp.tm_hour = (int)SavedPacket[4];
            payload.Timestamp.tm_min = (int)SavedPacket[5];
            payload.Timestamp.tm_sec = (int)SavedPacket[6];
            CanPacketSave.erase(CanID - frameID*3);
        }
    if(CanPacketSave.count(CanID - frameID*4))
        {
            memcpy(SavedPacket,CanPacketSave[CanID-frameID*4].c_str(),8);
            payload.connectorID = (int)SavedPacket[0];   
            payload.StopReason = SavedPacket[1]; 
            CanPacketSave.erase(CanID - frameID*4);      
        }
        String IDTag;
    for (size_t i = 0; i < IDtaglength; i++)
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
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.Timestamp.tm_year =((int)SavedPacket[0])*100 + (int)SavedPacket[1];
            payload.Timestamp.tm_mon = (int)SavedPacket[2];
            payload.Timestamp.tm_mday = (int)SavedPacket[3];
            payload.Timestamp.tm_hour = (int)SavedPacket[4];
            payload.Timestamp.tm_min = (int)SavedPacket[5];
            payload.Timestamp.tm_sec = (int)SavedPacket[6];
            CanPacketSave.erase(CanID);
        }
    if(CanPacketSave.count(CanID - frameID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID - frameID].c_str(),8);
            payload.connectorID = (int)SavedPacket[0];
            payload.ReasonError = SavedPacket[1];
            Payload_ErrorCnf ErrorCnf;
            Canpacket_ProtocolSend(ErrorCnf);
            CanPacketSave.erase(CanID - frameID);
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

void EVSEModelCan::CanProtocol_ChangeAvailabilityCnf(Payload_ChangeAvailabilityCnf& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.CAResstatus = SavedPacket[0];
            CanPacketSave.erase(CanID);
        }
    ChangeAvailabilityStatus = protocolChangeAvailabilityCnfStatus[(ChangeAvailabilityCnfStatus)payload.CAResstatus];
    //Serial.println(ChangeAvailabilityStatus);
}

void EVSEModelCan::CanProtocol_RemoteStartRes(Payload_RemoteStartRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.status = SavedPacket[0];
            CanPacketSave.erase(CanID);
        }
    RemoteStartStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(RemoteStartStatus);
}

void EVSEModelCan::CanProtocol_RemoteStopRes(Payload_RemoteStopRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.status = SavedPacket[0];
            CanPacketSave.erase(CanID);
        }
    RemoteStopStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(RemoteStopStatus);
}

void EVSEModelCan::CanProtocol_ResetRes(Payload_ResetRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.status = SavedPacket[0];
            CanPacketSave.erase(CanID);
        }
    ResetStatus = protocolCommonStatus[(CommonStatus)payload.status];
    //Serial.println(ResetStatus);
}

void EVSEModelCan::CanProtocol_UnlockConnectorRes(Payload_UnlockConnectorRes& payload){
    uint32_t CanID = *newProtocolCommand[payload.CmdID];
    char SavedPacket[8];
    if(CanPacketSave.count(CanID))
        {
            memcpy(SavedPacket,CanPacketSave[CanID].c_str(),8);
            payload.StatusUnlockConnector = SavedPacket[0];
            CanPacketSave.erase(CanID);
        }
    UnlockconnectorStatus = protocolUnlockConnectorStatus[(UnlockConnectorStatus)payload.StatusUnlockConnector];
    //Serial.println(UnlockconnectorStatus);
}
/*
void EVSEModelCan::CanProtocol_ChangeAvailabilityNtf(Payload_ChangeAvailabilityNtf& payload){
    for (int i = 0; i < CPSS->getNumConnectors(); i++) {
        if(CPSS->getConnector(i)->getAvailability()!=SavedAvailablityStatus[i]){
        payload.connectorID = i;
        payload.CAReqstatus = CAS_Accepted;//

        uint32_t CanID = *newProtocolCommand[payload.CmdID];
        CAN.beginExtendedPacket(CanID, 8);
        CAN.write(payload.connectorID);
        CAN.write(payload.CAReqstatus);
        CAN.endPacket();
        
        SavedAvailablityStatus[i] = CPSS->getConnector(i)->getAvailability();
        }
    }
}
*/
String EVSEModelCan::getchargePointModel(){
    return chargePointModel;
}

int EVSEModelCan::getconnectorNum(){
    return connectorNum;
}

String EVSEModelCan::getchargePointSerialNumber(){
    return chargePointSerialNumber;
}

String EVSEModelCan::getchargePointVendor(){
    return chargePointVendor;
}

String EVSEModelCan::getfirmwareVersion(){
    return firmwareVersion;
}  

uint8_t EVSEModelCan::getCablestatus(){
    return Cablestatus;
}

uint8_t EVSEModelCan::getCPstatus(){
    return CPstatus;
}

uint8_t EVSEModelCan::getlockstutus(){
    return lockstutus;
}

float EVSEModelCan::getL1Voltage(){
    return L1Voltage;
} 

float EVSEModelCan::getL1current(){
    return L1Current;
} 

float EVSEModelCan::getL1Power(){
    return L1Power;
}

float EVSEModelCan::getL2Voltage(){
    return L2Voltage;
} 

float EVSEModelCan::getL2current(){
    return L2Current;
} 

float EVSEModelCan::getL2Power(){
    return L2Power;
}

float EVSEModelCan::getL3Voltage(){
    return L3Voltage;
} 

float EVSEModelCan::getL3current(){
    return L3Current;
} 

float EVSEModelCan::getL3Power(){
    return L3Power;
}

String EVSEModelCan::getErrorCode(){
    return ErrorCode;
}

float EVSEModelCan::getMeterStop(){
    return MeterStop;
}
    
tm EVSEModelCan::getMeterValueTimestamp(){
    return MeterValueTimestamp;    
}

tm EVSEModelCan::getStopChargingTimestamp(){
    return StopChargingTimestamp;
}

tm EVSEModelCan::getErrorTimestamp(){
    return ErrorTimestamp;
}

String EVSEModelCan::getAuthorizeidtag(){
    return Authorizeidtag;
}

String EVSEModelCan::getStopChargingidtag(){
    return StopChargingidtag;
}

int EVSEModelCan::getAuthorizeCID(){
    return AuthorizeCID;
}

int EVSEModelCan::getHeartbeatCID(){
    return HeartbeatCID;
}

int EVSEModelCan::getMeterValueCID(){
    return MeterValueCID;
}

int EVSEModelCan::getStopChargingCID(){
    return StopChargingCID;
}

int EVSEModelCan::getErrorCID(){
    return ErrorCID;
}
    
String EVSEModelCan::getStopReason(){
    return StopReason;
}

String EVSEModelCan::getChangeAvailabilityStatus(){
    return ChangeAvailabilityStatus;
}

String EVSEModelCan::getRemoteStartStatus(){
    return RemoteStartStatus;
}

String EVSEModelCan::getRemoteStopStatus(){
    return RemoteStopStatus;
}
    
String EVSEModelCan::getResetStatus(){
    return ResetStatus;
}

String EVSEModelCan::getUnlockconnectorStatus(){
    return UnlockconnectorStatus;
}

float EVSEModelCan::getTotalElectricity(){
    return TotalElectricity;
}

int EVSEModelCan::getIDtaglength(){
    return IDtaglength;
}

String EVSEModelCan::HexToAscii(char HexValue)
{    
    uint8_t HexTop, HexBtm;
    uint8_t AsciiTop,AsciiBtm;

    HexTop = HexValue / 16;
    HexBtm = HexValue % 16;
    
	if((HexTop>=0)&&(HexTop<=9))	        AsciiTop = HexTop + 0x30;
	else if((HexTop>=10)&&(HexTop<=15))     AsciiTop = HexTop + 0x37;
	else                                    AsciiTop = 0xff;
		
	if((HexBtm>=0)&&(HexBtm<=9))	        AsciiBtm = HexBtm + 0x30;
	else if((HexBtm>=10)&&(HexBtm<=15))     AsciiBtm = HexBtm + 0x37;
	else                                    AsciiBtm = 0xff;
	char temp[2];
    //Serial.println((char)AsciiTop);
    //Serial.println((char)AsciiBtm);
    temp[0]=(char)AsciiTop;
    temp[1]=(char)AsciiBtm;
    String str;
    for (size_t i = 0; i < 2; i++)
    {
        str += temp[i];
    }
     
    //str = temp;
    //Serial.println(str);
	return str;
}

EVSEModelCan  Canmodel;