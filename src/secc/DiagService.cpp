#include "DiagService.h"
#include "task/event_log.h"
#include "MongooseHttpClient.h"
using namespace ArduinoOcpp;

dia::dia(EventLog &eventLog)
{
    this->eventLog = &eventLog;
}

void dia::begin(){
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

