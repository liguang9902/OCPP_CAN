#ifndef NEWPROTOCOL_HPP
#define NEWPROTOCOL_HPP

#include "ProtocolEVSE.hpp"
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
extern const int newProtocolCommand[ProtocolCommand_MAX][1];

typedef enum{
    AS_Accepted,       //接受标识符
    AS_Blocked,        //标识符被阻止
    AS_ConcurrentTx,   //标识符已经启动了一项事务，且不允许多个事务
    AS_Expired,        //标识符已过期
    AS_Invalid         //标识符无效的
}AuthorizeStatus;

typedef enum{
    BS_Accepted,
    BS_Rejected
}BootStatus;

typedef enum{
    plugged,
    unplugged
}CableStatus;

typedef enum{
    CP_Status_A1,		
	CP_Status_B1,		
	CP_Status_C1,		
	CP_Status_A2,		
	CP_Status_B2,		
	CP_Status_C2,		
	CP_Status_E,
    CP_Status_F
}CPStatus;

typedef enum{
    locked,
    unlocked
}LockStatus;

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

typedef enum{
    Inoperative,
    operative
}ChangeAvailabilityReqStatus;

typedef enum{
    CAS_Accepted,
    CAS_Rejectedcked,
    CAS_Scheduled
}ChangeAvailabilityResStatus;

typedef enum{
    Accepted,
    Rejected
}CommonStatus;

typedef enum{
    Unlocked,
    UnlockFailed
}UnlockConnectorStatus;

#endif