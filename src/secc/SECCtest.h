#ifndef SECCTEST_H
#define SECCTEST_H

#include "secc/ProtocolEVSE.hpp"
#include "secc/evseInterface.hpp"

COMM_ERROR_E receiveRGettime(ResponsePayloadEVSE_getTime &resGettime);
COMM_ERROR_E receiveRSettime(ResponsePayloadEVSE_setTime &resGettime);
COMM_ERROR_E receiveRgetEVSEState(ResponsePayloadEVSE_getEVSEState &resGetEvseState);
COMM_ERROR_E receiveRresGetRfidState(ResponsePayloadEVSE_getRFIDState &resGetRfidState);
COMM_ERROR_E receiveRgetEVSEState1(ResponsePayloadEVSE_getEVSEState &resGetEvseState);
COMM_ERROR_E receiveRgetEVSEState2(ResponsePayloadEVSE_getEVSEState &resGetEvseState);
#endif