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

EnrbootNotification::EnrbootNotification(const char *cpModel, const char *cpSerialNumber, const char *cpVendor, const char *fwVersion, const char *cbSerialNumber,
                const char *iccid, const char *imsI, const char *meterSerialNumber, const char *meterType){
    snprintf(chargePointModel, CP_MODEL_LEN_MAX + 1, "%s", cpModel);
    snprintf(chargePointSerialNumber, CP_SERIALNUMBER_LEN_MAX + 1, "%s", cpSerialNumber);
    snprintf(chargePointVendor, CP_VENDOR_LEN_MAX + 1, "%s", cpVendor);
    snprintf(firmwareVersion, FW_VERSION_LEN_MAX + 1, "%s", fwVersion);
    snprintf(chargeBoxSerialNumber, CP_BOXNUMBER_LEN_MAX + 1, "%s", cbSerialNumber);
    snprintf(ICCID, ICCID_LEN_MAX + 1, "%s", iccid);
    snprintf(IMSI, IMSI_LEN_MAX + 1, "%s", imsI);
    snprintf(MeterSerialNumber, METER_SERIALNUMBER_LEN_MAX + 1, "%s", meterSerialNumber);
    snprintf(MeterType, METER_TYPE_LEN_MAX + 1, "%s", meterType);
}

std::unique_ptr<DynamicJsonDocument> EnrbootNotification::createReq() {

    if (overridePayload != nullptr) {
        auto result = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(*overridePayload));
        return result;
    }

    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(9)
        + strlen(chargePointModel) + 1
        + strlen(chargePointVendor) + 1
        + strlen(chargePointSerialNumber) + 1
        + strlen(firmwareVersion) + 1
        + strlen(chargeBoxSerialNumber)+1
        + strlen(ICCID)+1
        + strlen(IMSI)+1
        + strlen(MeterSerialNumber)+1
        + strlen(MeterType)+1) );
    JsonObject payload = doc->to<JsonObject>();
    payload["chargePointModel"] = chargePointModel;
    if (chargePointSerialNumber[0]) {
        payload["chargePointSerialNumber"] = chargePointSerialNumber;
    }
    payload["chargePointVendor"] = chargePointVendor;
    if (firmwareVersion[0]) {
        payload["firmwareVersion"] = firmwareVersion;
    }
    if (chargeBoxSerialNumber[0]) {
        payload["chargeBoxSerialNumber"] = chargeBoxSerialNumber;
    }
    if (ICCID[0]) {
        payload["iccid"] = ICCID;
    }
    if (IMSI[0]) {
        payload["imsi"] = IMSI;
    }
    if (MeterSerialNumber[0]) {
        payload["meterSerialNumber"] = MeterSerialNumber;
    }
    if (MeterType[0]) {
        payload["meterType"] = MeterType;
    }
    return doc;
}