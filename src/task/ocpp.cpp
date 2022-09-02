#include "ocpp.h"

#include "core/NewOCPP.h"
#include <ArduinoOcpp/SimpleOcppOperationFactory.h>
#include "core/EnrEngine.h"
#include <ArduinoOcpp/Platform.h>
#include "task/EVSEModel.h"
#include "emChargePoint.hpp"
#include "MicroTasks.h"
#include "MicroDebug.h"
//#include "http_update.h"
#include "HTTPUpdate.h"

using namespace MicroTasks;
using namespace ArduinoOcpp;

OcppTask *OcppTask::instance = NULL;

OcppTask::OcppTask() : MicroTasks::Task() {
    ESP_LOGI(TAG_EMSECC,"create a model success");
}

OcppTask::~OcppTask() {
    instance = NULL;
}

void OcppTask::setup(){

}

void OcppTask::begin(EVSEModel *evse,EventLog &eventLog)
{
    this->evse = evse;
    this->eventLog = &eventLog;
    initializeArduinoOcpp();

    instance = this; 
    MicroTask.startTask(this);
}

void OcppTask::initializeArduinoOcpp() {
    uint64_t mac = ESP.getEfuseMac();
    String cpSerialNum = String((unsigned long)mac , 16);
    String cpModel = String(CP_Model);
    String cpVendor = String(CP_Vendor);    
    String csUrl =  String(OCPP_URL)+cpVendor+'_'+cpModel+'_'+cpSerialNum ;

    String ocppHost = OCPP_HOST;
    const char *ocpphost = ocppHost.c_str();
    OCPP_initialize(ocpphost, OCPP_PORT, csUrl.c_str());

    
        //endSession();
    
    ArduinoOcpp::ChargePointStatusService *CPSS = getChargePointStatusService();
    if (CPSS &&  CPSS->getConnector(0)) {
        CPSS->getConnector(0)->setAvailability(true);
        CPSS->getConnector(1)->endSession();
    Serial.println(F(" Set Connector Availability."));
             }
    
    
    loadEvseBehavior();
    initializeDiagnosticsService();
    initializeFwService();
    
    //uint64_t mac = ESP.getEfuseMac();
    //String cpSerialNum = String((unsigned long)mac , 16);

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
        payload["chargePointSerialNumber"] = cpSerialNum;
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
    

   onIdTagInput = [this] (const String& idInput) {
        if (idInput.isEmpty()) {
            ESP_LOGD(TAG_EMSECC,"[ocpp] empty idTag");
            return true;
        }
        if (!isAvailable() || !OcppInitialized) {
            
            ESP_LOGD(TAG_EMSECC,"[ocpp] present card but inoperative");
            return true;
        }
        const char *sessionIdTag = getSessionIdTag();
        if (sessionIdTag) {
            //currently in an authorized session
            if (idInput.equals(sessionIdTag)) {
                //NFC card matches
                endSession();
                ESP_LOGI(TAG_EMSECC,"Card accepted");
            } else {
                ESP_LOGW(TAG_EMSECC,"Card not recognized");
            }
        } else {
            //idle mode
            ESP_LOGI(TAG_EMSECC,"Card read");
            String idInputCapture = idInput;
            authorize(idInput.c_str(), [this, idInputCapture] (JsonObject payload) {
                if (idTagIsAccepted(payload)) {
                    beginSession(idInputCapture.c_str());
                    ESP_LOGI(TAG_EMSECC,"Card accepted");
                } else {
                    ESP_LOGW(TAG_EMSECC,"Card not recognized");
                }
            }, [this] () {
                ESP_LOGD(TAG_EMSECC,"OCPP timeout");
            });
        }

        return true;
    };*/

    //rfid->setOnCardScanned(&onIdTagInput);


    setOnRemoteStartTransactionSendConf([this](JsonObject payload){
        const char *type = payload["type"];


    });

    setOnResetReceiveReq([this] (JsonObject payload) {
        const char *type = payload["type"] | "Soft";
        if (!strcmp(type, "Hard")) {
            resetHard = true;
        }

        resetTime = millis();
        resetTriggered = true;

        ESP_LOGI(TAG_EMSECC,"Reboot EVSE");
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
                
                ESP_LOGI(TAG_EMSECC,"[ocpp] Diagnostics upload, invalid URL: %s",location.c_str());
                diagFailure = true;
                return false;
            }

            /*if (eventLog == NULL) {
                diagFailure = true;
                return false;
            }*/

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
                        
                        ESP_LOGD(TAG_EMSECC,"[ocpp] Diagnostics upload, cannot parse timestamp format:%s ",time);
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

            
            ESP_LOGI(TAG_EMSECC,"[ocpp] POST diagnostics file to %s",location.c_str());

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

        fwService->setOnInstall([this ,fwService](const std::string &location) {
                String location1 = location.c_str();
                ESP_LOGD(TAG_PROXY, "UpgradeProxy Start Install firmware , URL:[%s]" , location.c_str());
                String fileName = location1.substring(location1.lastIndexOf('/')+1);
                if(fileName.startsWith("LPC")){
                 ESP_LOGD(TAG_PROXY, "Notify URL is  a firmware for LPC ,filename=%s  ,pass...(Exist?=%s) " , fileName.c_str() , USE_FS.exists('/'+fileName.c_str())?"True":"False");
                 //proxyInstallationStatus = InstallationStatus::Installed;
                    return true;
                }

                WiFiClient client;
                //WiFiClientSecure client;
                //client.setCACert(rootCACertificate);
                client.setTimeout(60); //in seconds
    
                // httpUpdate.setLedPin(LED_BUILTIN, HIGH);
                t_httpUpdate_return ret = httpUpdate.update(client, location.c_str());

                switch (ret) {
                case HTTP_UPDATE_FAILED:
                fwService->setInstallationStatusSampler([](){return InstallationStatus::InstallationFailed;});
                //proxyInstallationStatus = InstallationStatus::InstallationFailed;
                 Serial.printf("[main] HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    break;
                case HTTP_UPDATE_NO_UPDATES:
                    fwService->setInstallationStatusSampler([](){return InstallationStatus::InstallationFailed;});
                    //proxyInstallationStatus = InstallationStatus::InstallationFailed;
                 Serial.println(F("[main] HTTP_UPDATE_NO_UPDATES"));
                    break;
                case HTTP_UPDATE_OK:
                    fwService->setInstallationStatusSampler([](){return InstallationStatus::Installed;});
                    //proxyInstallationStatus = InstallationStatus::Installed;
                    //installedFirmwareName = fileName;
                    Serial.println(F("[main] HTTP_UPDATE_OK"));

                ArduinoOcpp::ChargePointStatusService *CPSS = getChargePointStatusService();
                    if (CPSS &&  CPSS->getConnector(0)) {
                CPSS->getConnector(0)->setAvailability(true);
                    Serial.println(F("[FirmwareProxy] Set Connector Availability."));
                 }
                 sleep(2);
                 ESP.restart();
                    break;
                }   
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

unsigned long OcppTask::loop(MicroTasks::WakeReason reason){
    if (OcppInitialized) {
        OCPP_loop();
    }

    if (OcppInitialized) {
        
        if (evse->isVehicleConnected()) {
            //vehicle plugged
            
            if (!getSessionIdTag()) {
                //vehicle plugged before authorization
               
                  this->ocpp_idTag = "NTR";//由lpc传输获得 
                    //no RFID reader --> Auto-Authorize or RemoteStartTransaction
                    if (!ocpp_idTag.isEmpty() /*&& strcmp(tx_start_point.c_str(), "tx_only_remote")*/) {
                        //ID tag given in OpenEVSE dashboard & Auto-Authorize not disabled
                        String idTagCapture = ocpp_idTag;
                        authorize(idTagCapture.c_str(), [this, idTagCapture] (JsonObject payload) {
                            if (idTagIsAccepted(payload)) {
                                ESP_LOGI(TAG_EMSECC,"Session begin! idTag:%s \r\n",idTagCapture.c_str());
                                beginSession(idTagCapture.c_str());
                                transactionInitialized = true;
                            } else {
                                ESP_LOGD(TAG_EMSECC,"ID tag not recognized");
                            }
                        }, [this] () {
                            ESP_LOGD(TAG_EMSECC,"OCPP timeout");
                        });
                         //sleep(5);
                    } else {
                        //OpenEVSE cannot authorize this session -> wait for RemoteStartTransaction
                        ESP_LOGI(TAG_EMSECC,"Please authorize session");
                    }
                
            }
        }

        if (transactionInitialized && getTransactionId()<0)
        {
            ESP_LOGW(TAG_EMSECC, "Start new Transaction");
            startTransaction(this->ocpp_idTag.c_str(),
            [this](JsonObject conf)
            {
              string authConfirm;
              if (serializeJson(conf, authConfirm))
              {
                ESP_LOGD(TAG_EMSECC, "Start new Transaction confirm: %s \r\n", authConfirm.c_str());
              };
              if (conf.containsKey("idTagInfo"))
              {
                if (conf["idTagInfo"].containsKey("status"))
                {
                  //if( memcmp(conf["idTagInfo"] , "Accepted" ,8)==0 )
                  if (strcmp(conf["idTagInfo"]["status"], "Accepted") == 0)
                  {
                    this->transactionRunning = true;

                    sleep(5);// for test
                  }
                  else
                  {
                    //String retState ;

                    ESP_LOGW(TAG_EMSECC, "Transaction didn't accepted by CS: [?]"); // .as<const char *>()
                  }
                }
                else
                {
                  ESP_LOGW(TAG_EMSECC, "Transaction confirm missed key \"idTagInfo-status\"");
                }
              }
              else
              {
                ESP_LOGW(TAG_EMSECC, "Transaction confirm missed key \"idTagInfo\"");
              }
              
            });
            sleep(10);
        }
        
        if (this->transactionRunning)
        {
            ESP_LOGE(TAG_EMSECC, "Stop Transaction\n");
            stopTransaction(
            [](JsonObject confirm)
            {
            String confirmMessage;
            serializeJson(confirm, confirmMessage);
            ESP_LOGD(TAG_EMSECC, "Stop Transaction confirm:[%s]\r\n", confirmMessage.c_str());
            });
            this->transactionRunning = false;
            
        }
        

        vehicleConnected = evse->isVehicleConnected();

        if (ocppSessionDisplay && !getSessionIdTag()) {
            //Session unauthorized. Show if StartTransaction didn't succeed
            if (ocppTxIdDisplay < 0) {

                    ESP_LOGI(TAG_EMSECC,"Present card again");
               
                }
            }
            else if (!ocppSessionDisplay && getSessionIdTag()) {
            //Session recently authorized
            if (!evse->isVehicleConnected()) {
                ESP_LOGI(TAG_EMSECC,"Plug in cable");
            }
        }
        ocppSessionDisplay = getSessionIdTag();

        if (ocppTxIdDisplay <= 0 && getTransactionId() > 0) {
            ESP_LOGI(TAG_EMSECC,"OCPP start tx");
            String txIdMsg = "TxID ";
            txIdMsg += String(getTransactionId());
            ESP_LOGI(TAG_EMSECC,"%s",txIdMsg);
        } else if (ocppTxIdDisplay > 0 && getTransactionId() < 0) {
            ESP_LOGI(TAG_EMSECC,"OCPP Good bye!");
            String txIdMsg = "TxID ";
            txIdMsg += String(ocppTxIdDisplay);
            txIdMsg += " finished";
            ESP_LOGI(TAG_EMSECC,"%s",txIdMsg);
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
   return OcppInitialized ? 0 : 1000;
}