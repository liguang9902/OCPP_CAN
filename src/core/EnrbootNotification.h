#ifndef ENRBOOTNOTIFICATUIN_H
#define ENRBOOTNOTIFICATUIN_H

#include <ArduinoOcpp/MessagesV16/BootNotification.h>
#include <ArduinoOcpp/Core/OcppMessage.h>
#include <ArduinoOcpp/MessagesV16/CiStrings.h>

#define CP_MODEL_LEN_MAX        CiString20TypeLen
#define CP_SERIALNUMBER_LEN_MAX CiString25TypeLen
#define CP_VENDOR_LEN_MAX       CiString20TypeLen
#define FW_VERSION_LEN_MAX      CiString50TypeLen
#define CP_BOXNUMBER_LEN_MAX    CiString25TypeLen


namespace ArduinoOcpp {
namespace Ocpp16 {

class EnrbootNotification : public BootNotification
{
private:
    char chargePointModel [CP_MODEL_LEN_MAX + 1] = {'\0'};
    char chargePointSerialNumber [CP_SERIALNUMBER_LEN_MAX + 1] = {'\0'};
    char chargePointVendor [CP_VENDOR_LEN_MAX + 1] = {'\0'};
    char firmwareVersion [FW_VERSION_LEN_MAX + 1] = {'\0'};
    char chargeBoxSerialNumber [CP_BOXNUMBER_LEN_MAX+1] = {'\0'};

     DynamicJsonDocument *overridePayload = NULL;
public:
    EnrbootNotification(const char *chargePointModel, const char *chargePointSerialNumber, const char *chargePointVendor, const char *firmwareVersion, const char *chargeBoxSerialNumber);
     std::unique_ptr<DynamicJsonDocument> createReq();


};





}
}


#endif