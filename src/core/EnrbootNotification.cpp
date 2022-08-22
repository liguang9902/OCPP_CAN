#include "core/EnrbootNotification.h"
#include <ArduinoOcpp/Core/OcppModel.h>
#include <ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h>
#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/Debug.h>

#include <string.h>

using ArduinoOcpp::Ocpp16::EnrbootNotification;

EnrbootNotification::EnrbootNotification(const char *cpModel, const char *cpSerialNumber, const char *cpVendor, const char *fwVersion, const char *cbSerialNumber){
    snprintf(chargePointModel, CP_MODEL_LEN_MAX + 1, "%s", cpModel);
    snprintf(chargePointSerialNumber, CP_SERIALNUMBER_LEN_MAX + 1, "%s", cpSerialNumber);
    snprintf(chargePointVendor, CP_VENDOR_LEN_MAX + 1, "%s", cpVendor);
    snprintf(firmwareVersion, FW_VERSION_LEN_MAX + 1, "%s", fwVersion);
    snprintf(chargeBoxSerialNumber, CP_BOXNUMBER_LEN_MAX + 1, "%s", cbSerialNumber);
}

std::unique_ptr<DynamicJsonDocument> EnrbootNotification::createReq() {

    if (overridePayload != nullptr) {
        auto result = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(*overridePayload));
        return result;
    }

    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(5)
        + strlen(chargePointModel) + 1
        + strlen(chargePointVendor) + 1
        + strlen(chargePointSerialNumber) + 1
        + strlen(firmwareVersion) + 1
        + strlen(chargeBoxSerialNumber)+1) );
    JsonObject payload = doc->to<JsonObject>();
    payload["chargePointModel"] = chargePointModel;
    if (chargePointSerialNumber[0]) {
        payload["chargePointSerialNumber"] = chargePointSerialNumber;
    }
    payload["chargePointVendor"] = chargePointVendor;
    if (firmwareVersion[0]) {
        payload["firmwareVersion"] = firmwareVersion;
    }
    if (firmwareVersion[0]) {
        payload["chargeBoxSerialNumber"] = chargeBoxSerialNumber;
    }
    return doc;
}