#include "ocpp.h"

#include "core/NewOCPP.h"
#include <ArduinoOcpp/SimpleOcppOperationFactory.h>
#include "core/EnrEngine.h"
#include <ArduinoOcpp/Platform.h>
#include "task/EVSEModel.h"
#include "emChargePoint.hpp"
#include "MicroTasks.h"
#include "MicroDebug.h"
#include "http_update.h"

using namespace MicroTasks;
using namespace ArduinoOcpp;

OcppTask *OcppTask::instance = NULL;

OcppTask::OcppTask() : MicroTasks::Task() {

}

OcppTask::~OcppTask() {

}

void OcppTask::begin(EVSEModel &evse)
{
    this->evse = &evse;
    
    initializeArduinoOcpp();

    instance = this; 
    MicroTask.startTask(this);
}

void OcppTask::initializeArduinoOcpp() {

    loadEvseBehavior();
    initializeDiagnosticsService();
    initializeFwService();
    
    uint64_t mac = ESP.getEfuseMac();
    String cpSerialNum = String((unsigned long)mac , 16);

    DynamicJsonDocument *evseDetailsDoc = new DynamicJsonDocument((JSON_OBJECT_SIZE(9)
        + strlen(CP_Model) + 1
        + strlen(CP_Vendor) + 1
        + strlen(cpSerialNum.c_str()) + 1
        + strlen(FWVersion) + 1
        + strlen(CBSerialNum)+1
        + strlen(ICCID)+1
        + strlen(IMSI)+1
        + strlen(MSerialNumber)+1
        + strlen(MType)+1) );
    JsonObject payload = evseDetailsDoc->to<JsonObject>();
    payload["chargePointModel"] = CP_Model;
    if (cpSerialNum[0]) {
        payload["chargePointSerialNumber"] = cpSerialNum.c_str();
    }
    payload["chargePointVendor"] = CP_Vendor;
    if (FWVersion[0]) {
        payload["firmwareVersion"] = FWVersion;
    }
    if (CBSerialNum[0]) {
        payload["chargeBoxSerialNumber"] = CBSerialNum;
    }
    if (ICCID[0]) {
        payload["iccid"] = ICCID;
    }
    if (IMSI[0]) {
        payload["imsi"] = IMSI;
    }
    if (MSerialNumber[0]) {
        payload["meterSerialNumber"] = MSerialNumber;
    }
    if (MType[0]) {
        payload["meterType"] = MType;
    }

    bootNotification(evseDetailsDoc, [this](JsonObject payload) { 
        ESP_LOGI(TAG_EMSECC, "ready. Wait for BootNotification.conf(), then start\n");
    });
    ocppTxIdDisplay = getTransactionId();
    ocppSessionDisplay = getSessionIdTag();

    OcppInitialized = true;
}

void OcppTask::loadEvseBehavior(){
    //从evse处得到数据并且初始化
    setPowerActiveImportSampler([this]() {
        return (float) (evse->getAmps() * evse->getVoltage());
    });

    setEnergyActiveImportSampler([this] () {
        float activeImport = 0.f;
        activeImport += (float) evse->getTotalEnergy();
        activeImport += (float) evse->getSessionEnergy();
        return activeImport;
    });

    setOnChargingRateLimitChange([this] (float limit) { //limit = maximum charge rate in Watts
        charging_limit = limit;
    });

    setConnectorPluggedSampler([this] () {
        return (bool) evse->isVehicleConnected();
    });

    setEvRequestsEnergySampler([this] () {
        return (bool) evse->isCharging();
    });

    setConnectorEnergizedSampler([this] () {
        return evse->isActive();
    });

    /*
     * Report failures to central system. Note that the error codes are standardized in OCPP
     

    addConnectorErrorCodeSampler([this] () {
        if (evse->getEvseState() == OPENEVSE_STATE_GFI_FAULT ||
                evse->getEvseState() == OPENEVSE_STATE_GFI_SELF_TEST_FAILED ||
                evse->getEvseState() == OPENEVSE_STATE_NO_EARTH_GROUND ||
                evse->getEvseState() == OPENEVSE_STATE_DIODE_CHECK_FAILED) {
            return "GroundFailure";
        }
        return (const char *) NULL;
    });

    addConnectorErrorCodeSampler([this] () {
        if (evse->getEvseState() == OPENEVSE_STATE_OVER_TEMPERATURE) {
            return "HighTemperature";
        }
        return (const char *) NULL;
    });

    addConnectorErrorCodeSampler([this] () {
        if (evse->getEvseState() == OPENEVSE_STATE_OVER_CURRENT) {
            return "OverCurrentFailure";
        }
        return (const char *) NULL;
    });

    addConnectorErrorCodeSampler([this] () {
        if (evse->getEvseState() == OPENEVSE_STATE_STUCK_RELAY) {
            return "PowerSwitchFailure";
        }
        return (const char *) NULL;
    });

    addConnectorErrorCodeSampler([this] () {
        if (rfid->communicationFails()) {
            return "ReaderFailure";
        }
        return (const char *) nullptr;
    });
    */

   onIdTagInput = [this] (const String& idInput) {
        if (idInput.isEmpty()) {
            DBUGLN("[ocpp] empty idTag");
            return true;
        }
        if (!isAvailable() || !OcppInitialized) {
            
            DBUGLN(F("[ocpp] present card but inoperative"));
            return true;
        }
        const char *sessionIdTag = getSessionIdTag();
        if (sessionIdTag) {
            //currently in an authorized session
            if (idInput.equals(sessionIdTag)) {
                //NFC card matches
                endSession();
                DBUGLN("Card accepted");
            } else {
                DBUGLN("Card not recognized");
            }
        } else {
            //idle mode
            DBUGLN("Card read");
            String idInputCapture = idInput;
            authorize(idInput.c_str(), [this, idInputCapture] (JsonObject payload) {
                if (idTagIsAccepted(payload)) {
                    beginSession(idInputCapture.c_str());
                    DBUGLN("Card accepted");
                } else {
                    DBUGLN("Card not recognized");
                }
            }, [this] () {
                DBUGLN("OCPP timeout");
            });
        }

        return true;
    };

    //rfid->setOnCardScanned(&onIdTagInput);

    setOnResetReceiveReq([this] (JsonObject payload) {
        const char *type = payload["type"] | "Soft";
        if (!strcmp(type, "Hard")) {
            resetHard = true;
        }

        resetTime = millis();
        resetTriggered = true;

        DBUGLN("Reboot EVSE");
    });

    setOnUnlockConnector([] () {
        //TODO Send unlock command to peripherals. If successful, return true, otherwise false
        //see https://github.com/OpenEVSE/ESP32_WiFi_V4.x/issues/230
        return false;
    });
}

void OcppTask::initializeDiagnosticsService(){
ArduinoOcpp::DiagnosticsService *diagService = getDiagnosticsService();
    if (diagService) {
        diagService->setOnUploadStatusSampler([this] () {
            if (diagFailure) {
                return ArduinoOcpp::UploadStatus::UploadFailed;
            } else if (diagSuccess) {
                return ArduinoOcpp::UploadStatus::Uploaded;
            } else {
                return ArduinoOcpp::UploadStatus::NotUploaded;
            }
        });

        diagService->setOnUpload([this] (const std::string &location, ArduinoOcpp::OcppTimestamp &startTime, ArduinoOcpp::OcppTimestamp &stopTime) {
            
            //reset reported state
            diagSuccess = false;
            diagFailure = false;

            //check if input URL is valid
            unsigned int port_i = 0;
            struct mg_str scheme, query, fragment;
            if (mg_parse_uri(mg_mk_str(location.c_str()), &scheme, NULL, NULL, &port_i, NULL, &query, &fragment)) {
                DBUG(F("[ocpp] Diagnostics upload, invalid URL: "));
                DBUGLN(location.c_str());
                diagFailure = true;
                return false;
            }

            if (eventLog == NULL) {
                diagFailure = true;
                return false;
            }

            //create file to upload
            #define BOUNDARY_STRING "-----------------------------WebKitFormBoundary7MA4YWxkTrZu0gW025636501"
            const char *bodyPrefix PROGMEM = BOUNDARY_STRING "\r\n"
                    "Content-Disposition: form-data; name=\"file\"; filename=\"diagnostics.log\"\r\n"
                    "Content-Type: application/octet-stream\r\n\r\n";
            const char *bodySuffix PROGMEM = "\r\n\r\n" BOUNDARY_STRING "--\r\n";
            const char *overflowMsg PROGMEM = "{\"diagnosticsMsg\":\"requested search period exceeds maximum diagnostics upload size\"}";

            const size_t MAX_BODY_SIZE = 10000; //limit length of message
            String body = String('\0');
            body.reserve(MAX_BODY_SIZE);
            body += bodyPrefix;
            body += "[";
            const size_t SUFFIX_RESERVED_AREA = MAX_BODY_SIZE - strlen(bodySuffix) - strlen(overflowMsg) - 2;

            bool firstEntry = true;
            bool overflow = false;
            for (uint32_t i = 0; i <= (eventLog->getMaxIndex() - eventLog->getMinIndex()) && !overflow; i++) {
                uint32_t index = eventLog->getMinIndex() + i;

                eventLog->enumerate(index, [this, startTime, stopTime, &body, SUFFIX_RESERVED_AREA, &firstEntry, &overflow] (String time, EventType type, const String &logEntry, EvseState managerState, uint8_t evseState, uint32_t evseFlags, uint32_t pilot, double energy, uint32_t elapsed, double temperature, double temperatureMax, uint8_t divertMode, uint8_t shaper) {
                    if (overflow) return;
                    ArduinoOcpp::OcppTimestamp timestamp = ArduinoOcpp::OcppTimestamp();
                    if (!timestamp.setTime(time.c_str())) {
                        DBUG(F("[ocpp] Diagnostics upload, cannot parse timestamp format: "));
                        DBUGLN(time);
                        return;
                    }

                    if (timestamp < startTime || timestamp > stopTime) {
                        return;
                    }

                    if (body.length() + logEntry.length() + 10 < SUFFIX_RESERVED_AREA) {
                        if (firstEntry)
                            firstEntry = false;
                        else
                            body += ",";
                        
                        body += logEntry;
                        body += "\n";
                    } else {
                        overflow = true;
                        return;
                    }
                });
            }

            if (overflow) {
                if (!firstEntry)
                    body += ",\r\n";
                body += overflowMsg;
            }

            body += "]";

            body += bodySuffix;

            DBUG(F("[ocpp] POST diagnostics file to "));
            DBUGLN(location.c_str());

            MongooseHttpClientRequest *request =
                    diagClient.beginRequest(location.c_str());
            request->setMethod(HTTP_POST);
            request->addHeader("Content-Type", "multipart/form-data; boundary=" BOUNDARY_STRING);
            request->setContent(body.c_str());
            request->onResponse([this] (MongooseHttpClientResponse *response) {
                if (response->respCode() == 200) {
                    diagSuccess = true;
                } else {
                    diagFailure = true;
                }
            });
            request->onClose([this] () {
                if (!diagSuccess) {
                    //triggered onClose before onResponse
                    diagFailure = true;
                }
            });
            diagClient.send(request);
            
            return true;
        });
    }
}

void OcppTask::initializeFwService(){
ArduinoOcpp::FirmwareService *fwService = getFirmwareService();
    if (fwService) {
        fwService->setBuildNumber(evse->getFirmwareVersion());

        fwService->setInstallationStatusSampler([this] () {
            if (updateFailure) {
                return ArduinoOcpp::InstallationStatus::InstallationFailed;
            } else if (updateSuccess) {
                return ArduinoOcpp::InstallationStatus::Installed;
            } else {
                return ArduinoOcpp::InstallationStatus::NotInstalled;
            }
        });

        fwService->setOnInstall([this](const std::string &location) {

            DBUGLN(F("[ocpp] Starting installation routine"));
            
            //reset reported state
            updateFailure = false;
            updateSuccess = false;

            return http_update_from_url(String(location.c_str()), [] (size_t complete, size_t total) { },
                [this] (int status_code) {
                    //onSuccess
                    updateSuccess = true;

                    resetTime = millis();
                    resetTriggered = true;
                }, [this] (int error_code) {
                    //onFailure
                    updateFailure = true;
                });
        });
    }
}

/*void OcppTask::updateEvseClaim() {

    EvseState evseState;
    EvseProperties evseProperties;

    if (!OcppInitialized /*|| !config_ocpp_enabled()) {
        evse->release(EvseClient_OpenEVSE_Ocpp);
        return;
    }

    if (getTransactionId() == 0 /*&& !strcmp(tx_start_point.c_str(), "tx_pending")) {
        //transaction initiated but neither accepted nor rejected
        evseState = EvseState::Active;
    } else if (ocppPermitsCharge()) {
        evseState = EvseState::Active;
    } else {
        evseState = EvseState::Disabled;
    }

    evseProperties = evseState;

    //OCPP Smart Charging?
    if (charging_limit < 0.f) {
        //OCPP Smart Charging is off. Nothing to do
    } else if (charging_limit >= -0.001f && charging_limit < 5.f) {
        //allowed charge rate is "equal or almost equal" to 0W
        evseState = EvseState::Disabled; //override state
        evseProperties = evseState; //renew properties
    } else {
        //charge rate is valid. Set charge rate
        float volts = evse->getVoltage(); // convert Watts to Amps. TODO Maybe use "smoothed" voltage value?
        if (volts > 0) {
            float amps = charging_limit / volts;
            evseProperties.setChargeCurrent(amps);
        }
    }

    if (evseState == EvseState::Disabled /*&& !config_ocpp_access_can_suspend()) {
        //OCPP is configured to never put the EVSE into sleep
        evseState = EvseState::None;
        evseProperties = evseState;
    }

    if (evseState == EvseState::Active /*&& !config_ocpp_access_can_energize()) {
        //OCPP is configured to never override the sleep mode of other services
        evseState = EvseState::None;
        evseProperties = evseState;
    }

    //Apply inferred claim
    if (evseState == EvseState::None) {
        //the claiming rules don't specify the EVSE state
        evse->release(EvseClient_OpenEVSE_Ocpp);
    } else {
        //the claiming rules specify that the EVSE is either active or inactive
        evse->claim(EvseClient_OpenEVSE_Ocpp, EvseManager_Priority_Ocpp, evseProperties);
    }

}*/

bool OcppTask::idTagIsAccepted(JsonObject payload) {
    const char *status = payload["idTagInfo"]["status"] | "Invalid";
    return !strcmp(status, "Accepted");
}

void OcppTask::loop(){
    if (OcppInitialized) {
        OCPP_loop();
    }

    if (OcppInitialized) {
        
        if (evse->isVehicleConnected() && !vehicleConnected) {
            //vehicle plugged
            if (!getSessionIdTag()) {
                //vehicle plugged before authorization
                String ocpp_idTag = "NTR";//由lpc传输获得 
                    //no RFID reader --> Auto-Authorize or RemoteStartTransaction
                    if (!ocpp_idTag.isEmpty() /*&& strcmp(tx_start_point.c_str(), "tx_only_remote")*/) {
                        //ID tag given in OpenEVSE dashboard & Auto-Authorize not disabled
                        String idTagCapture = ocpp_idTag;
                        authorize(idTagCapture.c_str(), [this, idTagCapture] (JsonObject payload) {
                            if (idTagIsAccepted(payload)) {
                                beginSession(idTagCapture.c_str());
                            } else {
                                DBUGLN("ID tag not recognized");
                            }
                        }, [this] () {
                            DBUGLN("OCPP timeout");
                        });
                    } else {
                        //OpenEVSE cannot authorize this session -> wait for RemoteStartTransaction
                        DBUGLN("Please authorize session");
                    }
                
            }
        }
        vehicleConnected = evse->isVehicleConnected();

        if (ocppSessionDisplay && !getSessionIdTag()) {
            //Session unauthorized. Show if StartTransaction didn't succeed
            if (ocppTxIdDisplay < 0) {

                    DBUGLN("Present card again");
               
                }
            }
            else if (!ocppSessionDisplay && getSessionIdTag()) {
            //Session recently authorized
            if (!evse->isVehicleConnected()) {
                DBUGLN("Plug in cable");
            }
        }
        ocppSessionDisplay = getSessionIdTag();

        if (ocppTxIdDisplay <= 0 && getTransactionId() > 0) {
            DBUGLN("OCPP start tx");
            String txIdMsg = "TxID ";
            txIdMsg += String(getTransactionId());
            DBUGLN(txIdMsg);
        } else if (ocppTxIdDisplay > 0 && getTransactionId() < 0) {
            DBUGLN("OCPP Good bye!");
            String txIdMsg = "TxID ";
            txIdMsg += String(ocppTxIdDisplay);
            txIdMsg += " finished";
            DBUGLN(txIdMsg);
        }
        ocppTxIdDisplay = getTransactionId();
    }

    if (resetTriggered) {
        if (millis() - resetTime >= 10000UL) { //wait for 10 seconds after reset command to send the conf msg
            resetTriggered = false; //execute only once

            if (resetHard) {
                //TODO send reset command to all peripherals
                //see https://github.com/OpenEVSE/ESP32_WiFi_V4.x/issues/228
            }
            
            sleep(5);
            ESP.restart();
        }
    }

   /* if (millis() - updateEvseClaimLast >= 1009) {
        updateEvseClaimLast = millis();
        updateEvseClaim();
    }*/
   
}