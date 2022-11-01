#ifndef NEWPROTOCOL_HPP
#define NEWPROTOCOL_HPP

#include <stdint.h>
#include <iostream>
#include <WString.h>
#include <map>
//#include "ProtocolEVSE.hpp"

#define frameID 0x0010000
typedef enum
{   
    ProtocolCommand_Authorizereq,               //Authorize.req: 当EVSE 获取到刷卡信息后，将Tag ID 发送给OCPP-CM,请求授权。
    ProtocolCommand_AuthorizeRes,               //Authorize.res: OCPP-CM回复授权状态。
    ProtocolCommand_BootNtf,                    //Boot.ntf: 当EVSE 每次启动后，需要将桩的相关信息告知OCPP-CM。
    ProtocolCommand_BootCnf,                    //Boot.cnf: OCPP-CM回复当前时间、心跳和电表数据的发送周期、桩是否被后台接受。
    ProtocolCommand_HeartbeatReq,               //heartbeat.req: EVSE与OCPP-CM周期性交互心跳报文，周期大小从Boot.cnf 中获取。
    ProtocolCommand_HeartbeatRes,               //heartbeat.res: OCPP-CM回复心跳报文，报文中包括当前时间和充电状态。
    ProtocolCommand_MeterValueNtf,              //MeterValue.ntf: EVSE周期性发送电表参数告知OCPP-CM，周期大小从Boot.cnf 中获取。
    ProtocolCommand_MeterValueCnf,              //MeterValue.cnf: OCPP-CM回复确认信息，报文为空。
    ProtocolCommand_StopChargingNtf,            //StopCharging.ntf: 当EVSE停止充电后，需立刻向OCPP-CM发起通知。
    ProtocolCommand_StopChargingCnf,            //StopCharging.cnf: OCPP-CM回复确认信息，报文为idTag或为空。
    ProtocolCommand_ErrorNtf,                   //Error.ntf: 当EVSE出现故障时，需立刻向OCPP-CM发起通知，必要时同时进入停止充电流程。
    ProtocolCommand_ErrorCnf,                   //Error.cnf: OCPP-CM回复确认信息，报文为空。
    ProtocolCommand_ChangeAvailabilityReq,      //ChangeAvailability.req: OCPP-CM请求EVSE更改当前的可用状态。
    ProtocolCommand_ChangeAvailabilityRes,      //ChangeAvailability.res: EVSE回复是否接受状态改变。
    ProtocolCommand_RemoteStartReq,             //RemoteStart.req: OCPP-CM远程请求EVSE开始充电。
    ProtocolCommand_RemoteStartRes,             //RemoteStart.res: EVSE回复是否接受开始充电请求。
    ProtocolCommand_RemoteStopReq,              //RemoteStop.req: OCPP-CM远程请求EVSE停止充电。
    ProtocolCommand_RemoteStopRes,              //RemoteStop.res: EVSE回复是否接受停止充电。
    ProtocolCommand_ResetReq,                   //Reset.req: OCPP-CM请求EVSE重启，分为硬重启和软重启。
    ProtocolCommand_ResetRes,                   //Reset.res: EVSE回复是否接受重启。
    ProtocolCommand_UnlockConnectorReq,         //UnlockConnector.req: OCPP-CM请求EVSE解锁。
    ProtocolCommand_UnlockConnectorRes,         //UnlockConnector.res: EVSE回复解锁状态。
    ProtocolCommand_UpgradeReadyReq,            //UpgradeReady.req:OCPP-CM请求EVSE升级。
    ProtocolCommand_UpgradeReadyRes,            //UpgradeReady.res:EVSE回复能否进行升级。
    ProtocolCommand_UpgradingReq,               //Upgrading.req:OCPP-CM向传输EVSE升级文件。
    ProtocolCommand_UpgradingRes,               //Upgrading.res:EVSE向OCPP-CM回复是否收到文件
    ProtocolCommand_MAX,
}ProtocolCommand;
extern const uint32_t newProtocolCommand[ProtocolCommand_MAX][1];



typedef enum{
    AS_Accepted,       //接受标识符
    AS_Blocked,        //标识符被阻止
    AS_ConcurrentTx,   //标识符已经启动了一项事务，且不允许多个事务
    AS_Expired,        //标识符已过期
    AS_Invalid         //标识符无效的
}AuthorizeStatus;

typedef enum{
    AC,
    DC
}CPModel;
extern std::map<CPModel , String> protocolCPModel;

typedef enum{
    plugged,
    unplugged
}CableStatus;
extern std::map<CableStatus , String> protocolCableStatus;

typedef enum{
    CP_Status_A1 = 0xA1,		
	CP_Status_B1 = 0xB1,		
	CP_Status_C1 = 0xC1,		
	CP_Status_A2 = 0xA2,		
	CP_Status_B2 = 0xB2,		
	CP_Status_C2 = 0xC2,		
	CP_Status_E = 0x0E,
    CP_Status_F = 0x0F
}CPStatus;
extern std::map<CPStatus , String> protocolCPStatus;

typedef enum{
    locked,
    unlocked
}LockStatus;  //电子锁状态
extern std::map<LockStatus , String> protocolLockStatus;

typedef enum{
    Available, 
    Preparing,
    Charging,
    SuspendedEV,
    SuspendedEVSE,
    Finishing, 	
    Reserved, 		
    Unavailable,	
    Faulted 
}EVSEStatus;


typedef enum{
    Local,
    DeAuthorized,
    EmergencyStop,
    EVDisconnected,
    HardReset,
    Reboot,
    Remote,
    PoweLoss,
    SoftReset, 		
    UnlockCommand
}StopCharingReason;
extern std::map<StopCharingReason , String> protocolStopCharingReason;

typedef enum{
    ConnectorLockFailure,
    EVCommunicationError,
    GroundFailure,
    HighTemperature, 		
    InternalError, 		
    LocalListConflict, 
    OverCurrentFailure, 		
    OverVoltage,		
    PowerMeterFailure, 
    PowerSwitchFailure,
    ReaderFailure, 		
    UnderVoltage, 
    WeakSignal
}ErrorReason;
extern std::map<ErrorReason , String> protocolErrorReason;

typedef enum{
    Inoperative,
    operative
}ChangeAvailabilityReqStatus;

typedef enum{
    CAS_Accepted,
    CAS_Rejectedcked,
    CAS_Scheduled
}ChangeAvailabilityResStatus;
extern std::map<ChangeAvailabilityResStatus , String> protocolChangeAvailabilityResStatus;

typedef enum{
    Accepted,
    Rejected
}CommonStatus;
extern std::map<CommonStatus , String> protocolCommonStatus;

typedef enum{
    Unlocked,
    UnlockFailed,
    NotSupported
}UnlockConnectorStatus;
extern std::map<UnlockConnectorStatus , String> protocolUnlockConnectorStatus;

typedef enum{
    Hard,
    Soft
}ResetType;

#pragma pack(push) 
#pragma pack(1) 

struct Payload_AuthorizeReq
{
    ProtocolCommand CmdID = ProtocolCommand_Authorizereq;
    char Idtag[20] = {0,};
    uint8_t connectorID;
};

struct Payload_AuthorizeRes
{
    ProtocolCommand CmdID = ProtocolCommand_AuthorizeRes;
    uint8_t Authorizestatus;
};

struct Payload_BootNtf
{
    ProtocolCommand CmdID = ProtocolCommand_BootNtf;
    uint8_t CPModel;
    uint8_t connectorNum;
    char CPSerialNumber[26]={0,};
    char CPVender[20]={0,};
    char FimwareVersion[16]={0,};
};

struct Payload_BootCnf
{
    ProtocolCommand CmdID = ProtocolCommand_BootCnf;
    tm currentTime;
    uint8_t HeartBeatInterval;
    uint8_t MeterValueInterval;
    uint8_t BootStatus;
};


struct Payload_HeartbeatReq
{
    ProtocolCommand CmdID = ProtocolCommand_HeartbeatReq;
    uint8_t connectorID;
    uint8_t statusCB;
    uint8_t statusCP;
    uint8_t statusLock;
};

struct Payload_HeartbeatRes
{
    ProtocolCommand CmdID = ProtocolCommand_HeartbeatRes;
    tm currentTime;
    uint8_t statusEVSE;
};

struct Payload_MeterValueNtf
{
    ProtocolCommand CmdID = ProtocolCommand_MeterValueNtf;
    tm currentTime;
    uint8_t connectorID;
    float L1Voltage;
    float L1Current;
    float L1Power;
    float L2Voltage;
    float L2Current;
    float L2Power;
    float L3Voltage;
    float L3Current;
    float L3Power;
};

struct Payload_MeterValueCnf
{
    ProtocolCommand CmdID = ProtocolCommand_MeterValueCnf;

};

struct Payload_StopChargingNtf
{
    ProtocolCommand CmdID = ProtocolCommand_StopChargingNtf;
    char Idtag[20] = {0,};
    uint8_t connectorID;
    float MeterStop;
    uint8_t StopReason;
    tm Timestamp;
};

struct Payload_StopChargingCnf
{
    ProtocolCommand CmdID = ProtocolCommand_StopChargingCnf;

};

struct Payload_ErrorNtf
{
    ProtocolCommand CmdID = ProtocolCommand_ErrorNtf;
    tm Timestamp;
    uint8_t connectorID;
    uint8_t ReasonError;
};

struct Payload_ErrorCnf
{
    ProtocolCommand CmdID = ProtocolCommand_ErrorCnf;
};

struct Payload_ChangeAvailabilityReq
{
    ProtocolCommand CmdID = ProtocolCommand_ChangeAvailabilityReq;
    uint8_t connectorID;
    uint8_t CAReqstatus;
};

struct Payload_ChangeAvailabilityRes
{
    ProtocolCommand CmdID = ProtocolCommand_ChangeAvailabilityRes;
    uint8_t CAResstatus;
};

struct Payload_RemoteStartReq
{
    ProtocolCommand CmdID = ProtocolCommand_RemoteStartReq;
    char Idtag[20] = {0,};
    uint8_t connectorID;
};

struct Payload_RemoteStartRes
{
    ProtocolCommand CmdID = ProtocolCommand_RemoteStartRes;
    uint8_t status;
};

struct Payload_RemoteStopReq
{
    ProtocolCommand CmdID = ProtocolCommand_RemoteStopReq;
    uint8_t connectorID;
};

struct Payload_RemoteStopRes
{
    ProtocolCommand CmdID = ProtocolCommand_RemoteStopRes;
    uint8_t status;
};

struct Payload_ResetReq
{
    ProtocolCommand CmdID = ProtocolCommand_ResetReq;
    uint8_t type;
};

struct Payload_ResetRes
{
    ProtocolCommand CmdID = ProtocolCommand_ResetRes;
    uint8_t status;
};

struct Payload_UnlockConnectorReq
{
    ProtocolCommand CmdID = ProtocolCommand_UnlockConnectorReq;
    uint8_t connectorID;
};

struct Payload_UnlockConnectorRes
{
    ProtocolCommand CmdID = ProtocolCommand_UnlockConnectorRes;
    uint8_t StatusUnlockConnector;
};


typedef struct 
{
    uint32_t CommandID[1];
    uint8_t Payload[0];     //Append CRC16 at end
}NewPacket;

typedef struct 
{
    uint32_t CanID;
    uint8_t Payload[8]={0,};
}CanPacket;


#pragma pack(pop)

//template<typename T>
//void Canpacket_ProtocolRequest(T& payload ,  CanPacket& packet);

template<typename T>
void Canpacket_ProtocolSend(T& payload );

template<typename T>
void Canunpacket_ProtocolRes(T& payload );

#endif