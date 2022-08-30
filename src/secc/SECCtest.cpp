#include "secc/SECCtest.h"
#include "secc/ProtocolEVSE.hpp"
#include "secc/evseInterface.hpp"

COMM_ERROR_E receiveRGettime(ResponsePayloadEVSE_getTime &resGettime)
{
    resGettime.evseTime.tm_year = 2022;
    resGettime.evseTime.tm_mon  = 8 ;
    resGettime.evseTime.tm_mday = 29;
    resGettime.evseTime.tm_hour = 10;
    resGettime.evseTime.tm_min  = 50;
    resGettime.evseTime.tm_sec  = 32;
 
    return COMM_SUCCESS;
}

COMM_ERROR_E receiveRSettime(ResponsePayloadEVSE_setTime &resGettime)
{
    resGettime.evseTime.tm_year = 2022;
    resGettime.evseTime.tm_mon  = 8 ;
    resGettime.evseTime.tm_mday = 29;
    resGettime.evseTime.tm_hour = 15;
    resGettime.evseTime.tm_min  = 50;
    resGettime.evseTime.tm_sec  = 32;
 
    return COMM_SUCCESS;
}

COMM_ERROR_E receiveRgetEVSEState(ResponsePayloadEVSE_getEVSEState &resGetEvseState)
{
    resGetEvseState.stateCP = CP_STATE_C;
    resGetEvseState.statePP = PP_STATE_20A;
    resGetEvseState.stateLock = LOCK_STATE_MAX;
    return COMM_SUCCESS;
}


COMM_ERROR_E receiveRresGetRfidState(ResponsePayloadEVSE_getRFIDState &resGetRfidState)
{
    resGetRfidState.StateRFID = 8;
    resGetRfidState.rfid = "NTR";
    return COMM_SUCCESS;
}

COMM_ERROR_E receiveRgetEVSEState1(ResponsePayloadEVSE_getEVSEState &resGetEvseState)
{
    resGetEvseState.stateCP = CP_STATE_C;
    resGetEvseState.statePP = PP_STATE_20A;
    resGetEvseState.stateLock = LOCK_STATE_LOCKED;
    return COMM_SUCCESS;
}

COMM_ERROR_E receiveRgetEVSEState2(ResponsePayloadEVSE_getEVSEState &resGetEvseState)
{
    resGetEvseState.stateCP = CP_STATE_C;
    resGetEvseState.statePP = PP_STATE_20A;
    resGetEvseState.stateLock = LOCK_STATE_UNLOCKED;
    return COMM_SUCCESS;
}