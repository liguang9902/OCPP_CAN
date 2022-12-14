//#include <Variants.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WString.h>
//#include <FS.h>
#include <core/NewOCPP.h>
#include <ArduinoOcpp/Tasks/SmartCharging/SmartChargingService.h>
#include "LITTLEFS.h"
#include <esp_log.h>
#include "emFiniteStateMachine.hpp"
#include "ProtocolEVSE.hpp"
#include "evseInterface.hpp"
#include "emSECC.hpp"
#include "emChargePoint.hpp"
#include "FirmwareProxy.hpp"
//extern void esp_sync_sntp(struct tm& sysTimeinfo , int retryMax);
#include "Common.hpp"
#include "SECC_SPI.hpp"

using namespace std;
using namespace ArduinoOcpp;
using namespace ArduinoOcpp::Ocpp16;
#include <ArduinoOcpp/MessagesV16/GetConfiguration.h>
#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/Core/ConfigurationKeyValue.h>
#include <ArduinoOcpp/Core/OcppEngine.h>
#include <ArduinoOcpp/Tasks/FirmwareManagement/FirmwareService.h>
using ArduinoOcpp::Ocpp16::GetConfiguration;

#include "ArduinoOcpp/Core/OcppModel.h"
#include "ArduinoOcpp/Tasks/ChargePointStatus/ChargePointStatusService.h"

#include "secc/SECCtest.h"
#include "ArduinoOcpp/SimpleOcppOperationFactory.h"
#include "ArduinoOcpp/MessagesV16/DataTransfer.h"
//#include "task/event_log.h"
//#include "MongooseHttpClient.h"

//RS485IF MeterIF(35,33);
//long previousTime=0;
//long interval=16000;
EMSECC::EMSECC(SECC_SPIClass *pCommIF)
{
  this->evIsLock = false;
  this->evIsPlugged = false;
  this->evRequestsEnergy = false;

  this->seccFSM[SECC_State_Unknown] = &EMSECC::seccIdle;
  this->seccFSM[SECC_State_Initialize] = &EMSECC::seccInitialize;
  this->seccFSM[SECC_State_EvseLink] = &EMSECC::seccLinkEvse;
  this->seccFSM[SECC_State_BootOcpp] = &EMSECC::seccBootOcpp;
  this->seccFSM[SECC_State_Waitting] = &EMSECC::seccWaiting;
  this->seccFSM[SECC_State_Preparing] = &EMSECC::seccPreCharge;
  this->seccFSM[SECC_State_Charging] = &EMSECC::seccCharging;
  this->seccFSM[SECC_State_Finance] = &EMSECC::seccFinance;
  this->seccFSM[SECC_State_Finishing] = &EMSECC::seccStopCharge;

  this->seccFSM[SECC_State_maintaining] = &EMSECC::seccMaintaince;
  this->seccFSM[SECC_State_SuspendedEVSE] = &EMSECC::evseMalfunction;
  this->seccFSM[SECC_State_SuspendedSECC] = &EMSECC::seccMalfunction;

  esp_sync_sntp(sysTimeinfo, 5);

  emEVSE = new EVSE_Interfacer(pCommIF);
  setFsmState(SECC_State_Unknown, NULL);
}

EMSECC::EMSECC()
{

}

EMSECC::~EMSECC()
{
  //free(this->pRxBuffer);
  //this->setFsmState(SECC_State_Unknown, NULL);
}

void EMSECC::secc_setEVSE_PowerLimit(float limit)
{
  ESP_LOGD(TAG_EMSECC, "New charging limit Got set %f-->%f", this->powerLimit, limit);
  this->powerLimit = limit;
}

template <>
int EMSECC::Tfun<int>(int &value)
{
  return value + 1;
}

template <>
float EMSECC::Tfun<float>(float &value)
{
  return value + 100.0;
}

void EMSECC::getOcppConfiguration()
{
  //std::shared_ptr<std::vector<std::shared_ptr<AbstractConfiguration>>> configurationKeys;
  //configurationKeys = Ocpp16::getAllConfigurations();
  GetConfiguration getConfig;
  std::unique_ptr<DynamicJsonDocument> jsonConfig = getConfig.createConf();//111
  string strConfig;
  //serializeJson(*jsonConfig, Serial);
  serializeJson(*jsonConfig, strConfig);
  ESP_LOGD(TAG_EMSECC, "Current Configuration: %s\r\n", strConfig.c_str());
}

static SECC_State lastState = SECC_State_Unknown; //currentState
void EMSECC::secc_loop()
{
  //Common Step
  /*
    ??????????????????????????????????????????????????? ??? ?????? ????????????????????????
  */

  //?????????????????????????????????????????? ??? ?????????????????????????????????
  SECC_State currentState = this->getFsmState();
  if (currentState != lastState)
  {
    ESP_LOGI(TAG_EMSECC, "SECC finite-state machine [%s]-->[%s] \r\n", fsmName[lastState].c_str(), fsmName[currentState].c_str());
    lastState = currentState;
  }

  //this->fsmAction(NULL);
  if (seccFSM[currentState])
    (this->*(seccFSM[currentState]))(NULL);
  else
    ESP_LOGI(TAG_EMSECC, "SECC finite-state machine error , current state=%d [%s]\r\n", currentState, fsmName[currentState]);
}

void EMSECC::seccIdle(void *param)
{
  static uint32_t loopCount = 0;
  (void)param;
  int paraA = 1;
  if (!loopCount)
  {
    ESP_LOGD(TAG_EMSECC, "seccIdle %d", Tfun(paraA));
  }
  else
  {
    setFsmState(SECC_State_Initialize, NULL);
  }
  loopCount++;
}

void EMSECC::seccInitialize(void *param)
{
  static uint32_t loopCount = 0;
  (void)param;
  float paraA = 1.00;
  if (!loopCount)
  {
    ESP_LOGD(TAG_EMSECC, "seccInitialize %f", Tfun(paraA));
    loadEvseBehavior();
    //initializeDiagnosticsService();
    FirmwareService *firmwareService = getFirmwareService();
    firmwareService->setDownloadStatusSampler(this->proxyDownloadStatusSampler);
    //firmwareService->setOnDownload( this->proxyDownload );
    firmwareService->setOnDownload( 
      [secc = this](const std::string &location){
        String location1 = location.c_str();
        if (!FirmwareProxy::proxyDownload(location1))
        {
          return false;
        }
        else
        {
          if (FirmwareProxy::proxyDownloadStatusSampler() == DownloadStatus::DownloadFailed)
          {
            //impossible
            ESP_LOGE(TAG_EMSECC, "Firmware download success but DownloadStatus::DownloadFailed ,Error status!!" );
            return false;
          }
          else if (FirmwareProxy::proxyDownloadStatusSampler() == DownloadStatus::NotDownloaded)
          {
            ESP_LOGW(TAG_EMSECC, "Firmware download success but DownloadStatus::NotDownloaded , Not a LPC firmware . pass...!" );
            return true;
          } 
          else
          {
            String fileName = location1.substring(location1.lastIndexOf('/'));
            ESP_LOGW(TAG_EMSECC, "Firmware download success, LPC firmware[%s]" , fileName.c_str());
            SECC_State retState = secc->setFsmState(SECC_State_maintaining, fileName.c_str());
            ESP_LOGW(TAG_EMSECC, "SECC should be set to State:%s" , fsmName[retState].c_str());
            return true;
          }
        }
      });

      firmwareService->setInstallationStatusSampler(this->proxyInstallationStatusSampler);
      firmwareService->setOnInstall(this->proxyInstall);

      
      /*
      auto operation = makeOcppOperation();
      auto msg = std::unique_ptr<OcppMessage>{nullptr};
      //DataTransfer dataT("CustomVendor");
      //dataT.createReq();
      msg = std::unique_ptr<OcppMessage>(new DataTransfer("CustomVendor"));
      operation->setOcppMessage(std::move(msg));*/
  }
  else
  {
      setFsmState(SECC_State_EvseLink, NULL);
  }
  loopCount++;
  }

  void EMSECC::seccLinkEvse(void *param)
  {
    static uint32_t loopCount = 0;
    (void)param;
    //float paraA = 1.00;
    COMM_ERROR_E retCode = COMM_SUCCESS;
    if (!loopCount)
    {
      //--GetTime----------------------------------------------------------------------------------------------------------------------
      RequestPayloadEVSE_getTime reqGettime;
      retCode = emEVSE->sendRequest<RequestPayloadEVSE_getTime>(reqGettime);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Send Request_getTime error:%d(%s) while seccLinkEvse\r\n", retCode, ifCommErrorDesc[retCode].c_str());
        setFsmState(SECC_State_SuspendedSECC, NULL);
        return;
      };

      ResponsePayloadEVSE_getTime resGettime;
      //retCode = emEVSE->receiveResponse(resGettime);
      retCode = receiveRGettime(resGettime);

      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Receive Response_getTime error!\r\n");
        ESP_LOGE(TAG_EMSECC, "Receive Response_getTime error:%d(%s) while seccLinkEvse\r\n", retCode ,ifCommErrorDesc[retCode].c_str() );
        setFsmState(SECC_State_SuspendedEVSE, NULL);
        //setFsmState(SECC_State_BootOcpp, NULL);
        return;
      };
      ESP_LOGI(TAG_EMSECC, "EVSE_Interfacer receiveResponse :<ResponsePayloadEVSE_getTime> decode(%d):[%d-%d-%dT%d:%d:%d]\n",
               retCode, resGettime.evseTime.tm_year, resGettime.evseTime.tm_mon, resGettime.evseTime.tm_mday,
               resGettime.evseTime.tm_hour, resGettime.evseTime.tm_min, resGettime.evseTime.tm_sec);

      //--GetConfig----------------------------------------------------------------------------------------------------------------------
      getOcppConfiguration();

      RequestPayloadEVSE_getConfig reqGetConfig;
      retCode = emEVSE->sendRequest<RequestPayloadEVSE_getConfig>(reqGetConfig);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Send Request_getConfig error:%d(%s) while seccLinkEvse\r\n", retCode, ifCommErrorDesc[retCode].c_str());
        setFsmState(SECC_State_SuspendedSECC, NULL);
        return;
      };

      ResponsePayloadEVSE_getConfig resGetConfig;
      //retCode = emEVSE->receiveResponse(resGetConfig);
      retCode = COMM_SUCCESS;
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Receive Request_getConfig error:%d(%s) while seccLinkEvse\r\n", retCode, ifCommErrorDesc[retCode].c_str());
        setFsmState(SECC_State_SuspendedEVSE, NULL);
        return;
      };
      if (resGetConfig.powerLimit)
      {
        ESP_LOGD(TAG_EMSECC, "Receive configuration from EVSE  powerLimit = %f\r\n", resGetConfig.powerLimit);
        this->secc_setEVSE_PowerLimit(resGetConfig.powerLimit);
      }
    }
    else
    {
      setFsmState(SECC_State_BootOcpp, NULL);
    }
    loopCount++;
  }

  void EMSECC::seccBootOcpp(void *param)
  {
    static uint32_t loopCount = 0;
    (void)param;
    //float paraA = 1.00;
    //if (loopCount< 20000 && BootOcppFlag == false)
    if(!loopCount)
    {
      Payload_BootNtf bOOT;
      uint32_t CanID = *newProtocolCommand[bOOT.CmdID];
      //if(Canmodel.CanPacketSave[CanID- frameID*7]!=NULL){
      
      //if(Canmodel.BootFlag == true){
      uint64_t mac = ESP.getEfuseMac();
      String cpSerialNum = String((unsigned long)mac , 16);
      String cpModel = String(CP_Model);
      //String cpModel = Canmodel.getchargePointModel();
      //String cpSerialNum = Canmodel.getchargePointSerialNumber();
      //String cpVendor = Canmodel.getchargePointVendor();
      //String fwVersion = Canmodel.getfirmwareVersion();
      String cpVendor = String(CP_Vendor);    
      String csUrl =  String(OCPP_URL)+cpVendor+'_'+cpModel+'_'+cpSerialNum ;
      String fwVersion = String(FWVersion);
      String cbSerialNum = String(CBSerialNum);
      String iccid = String(ICCID);
      String imsi = String(IMSI);
      String meterSerialNumber = String(MSerialNumber);
      String meterType = String(MType);
      bootNotification(cpModel.c_str() , cpVendor.c_str() , cpSerialNum.c_str(), fwVersion.c_str(),cbSerialNum.c_str(),iccid.c_str(), imsi.c_str(), meterSerialNumber.c_str(),meterType.c_str(),
                        [this](JsonObject confMsg)
                       {
                         //This callback is executed when the .conf() response from the central system arrives
                         
                         ESP_LOGD(TAG_EMSECC, "BootNotification was answered. Central System clock: %s", confMsg["currentTime"].as<const char *>()); //as<string>()??????
                         this->evseIsBooted = true;
                         //esp_set_OcppTime(confMsg["currentTime"].as<const char *>());
                         Payload_BootCnf BootCnf;
                         //Canpacket_ProtocolSend(BootCnf,confMsg);
                       });
      loopCount++;
      ESP_LOGI(TAG_EMSECC, "ready. Wait for BootNotification.conf(), then start\n");
      BootOcppFlag = true;
      /*}
      else{
        ESP_LOGI(TAG_EMSECC, "Wait for Boot.ntf() from EVSE\n");
      }*/
    }

    if (!evseIsBooted)
    {
      loopCount++;
      sleep(1);
    }
    else
    {
      //--SetTime----------------------------------------------------------------------------------------------------------------------
      RequestPayloadEVSE_setTime reqSettime;
      COMM_ERROR_E retCode = emEVSE->sendRequest<RequestPayloadEVSE_setTime>(reqSettime);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Send Request_setTime error:%d(%s) while seccLinkEvse\r\n", retCode, ifCommErrorDesc[retCode].c_str());
        setFsmState(SECC_State_SuspendedSECC, NULL);
        return;
      };
      sleep(5);

      ResponsePayloadEVSE_setTime resSettime;
      //retCode = emEVSE->receiveResponse(resSettime);
      retCode = receiveRSettime(resSettime);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Receive Response_setTime error:%d(%s) while seccLinkEvse\r\n", retCode, ifCommErrorDesc[retCode].c_str());
        setFsmState(SECC_State_SuspendedEVSE, NULL);
        return;
      };
      ESP_LOGI(TAG_EMSECC, "EVSE_Interfacer receiveResponse :<ResponsePayloadEVSE_setTime> decode(%d):[%d-%d-%dT%d:%d:%d]\n",
               retCode, resSettime.evseTime.tm_year, resSettime.evseTime.tm_mon, resSettime.evseTime.tm_mday,
               resSettime.evseTime.tm_hour, resSettime.evseTime.tm_min, resSettime.evseTime.tm_sec);

      setFsmState(SECC_State_Waitting, NULL);
    };
  }

  void EMSECC::seccWaiting(void *param)
  {
    static uint32_t loopCount = 0;
    (void)param;
    COMM_ERROR_E retCode = COMM_SUCCESS;
    /*
    RequestPayloadEVSE_getEVSEState reqGetEvseState;
    retCode = emEVSE->sendRequest(reqGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Send Request_getEVSEState error:%d while seccWaiting\r\n", retCode);
      setFsmState(SECC_State_SuspendedSECC, NULL);
      return;
    };

    ResponsePayloadEVSE_getEVSEState resGetEvseState;
    //retCode = emEVSE->receiveResponse(resGetEvseState);
    retCode = receiveRgetEVSEState(resGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Receive Response_getEVSEState error:%d while seccWaiting\r\n", retCode);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      return;
    };*/
    uint8_t CPstatus = Canmodel.getCPstatus();
    switch (CPstatus)
    {
    case CP_Status_INVALID:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP error State:%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    case CP_Status_UNKNOWN:
      ESP_LOGW(TAG_EMSECC, "EVSE CP State Unknown! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      break;
    case CP_Status_A:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State unPlugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = false;
      break;
    case CP_Status_B:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State Plugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      break;
    case CP_Status_C:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State S2! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      this->evRequestsEnergy = true;
      break;
    default:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP State:%s(%d) , unspported!\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      //setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    }
    ESP_LOGD(TAG_EMSECC, "EVSEState [CP=%s(%d)] (RequestsEnergy=%d)",
             //PPStateName[resGetEvseState.statePP].c_str(), resGetEvseState.statePP,
             protocolCPStatus[(CPStatus)CPstatus], CPstatus, this->evRequestsEnergy);

    if ((this->authStatus.authState != STATE_AUTHORIZING) && (this->authStatus.authState != STATE_AUTHORIZE_SUCCESS))
    {
      /*
      RequestPayloadEVSE_getRFIDState reqGetRfidState;
      retCode = emEVSE->sendRequest(reqGetRfidState);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Send Request_getRFIDState error:%d while seccWaiting\r\n", retCode);
        setFsmState(SECC_State_SuspendedSECC, NULL);
        return;
      };

      ResponsePayloadEVSE_getRFIDState resGetRfidState;
      //retCode = emEVSE->receiveResponse(resGetRfidState);
      retCode = receiveRresGetRfidState(resGetRfidState);
      if (retCode != COMM_SUCCESS)
      {
        ESP_LOGE(TAG_EMSECC, "Receive Response_getRFIDState error:%d while seccWaiting\r\n", retCode);
        setFsmState(SECC_State_SuspendedEVSE, NULL);
        return;
      };
      //ESP_LOGD(TAG_EMSECC, "EVSEState [RFID:%d]", resGetRfidState.StateRFID);
      if (resGetRfidState.StateRFID == RFID_LENGTH_MAX)*/
      if(Canmodel.authorizeFlag == true)
      {
        authorizationProvided = true; //Deprecated
        //setAuthorizedStatus(resGetRfidState.StateRFID , resGetRfidState.rfid);
        //this->authStatus.authorizationProvided = (uint8_t)resGetRfidState.StateRFID;
        this->authStatus.rfidTag.clear();
        this->authStatus.rfidTag.concat(Canmodel.getAuthorizeidtag().c_str());
        ESP_LOGD(TAG_EMSECC, "EVSEState [RFID:(%d)%s]", Canmodel.getIDtaglength()*2, this->authStatus.rfidTag.c_str());
        Canmodel.authorizeFlag = false;
      }
      else
      {
        ESP_LOGW(TAG_EMSECC, "wait for Authorize.req!\r\n");
      }
    }

    switch (authStatus.authState)
    {
    case STATE_AUTHORIZE_UNKNOWN:

      if (authorizationProvided)
      {

        ESP_LOGD(TAG_EMSECC, "STATE_AUTHORIZE_UNKNOWN , send request to CS.[rfid=%s]\r\n", this->authStatus.rfidTag.c_str());
        authorizeState = STATE_AUTHORIZING;
        authStatus.authState = STATE_AUTHORIZING;
        authorize(
            this->authStatus.rfidTag.c_str(),
            [this](JsonObject confMsg) //Confirm
            {
              string authConfirm;
              if (serializeJson(confMsg, authConfirm))
              {
                ESP_LOGD(TAG_EMSECC, "Tag=%s Authorized response confirm: %s \r\n", authStatus.rfidTag.c_str(), authConfirm.c_str());
              };

              if (confMsg.containsKey("idTagInfo"))
              {
                //const char* idTagInfo_status = doc["idTagInfo"]["status"]; // "Accepted"
                //const char* idTagInfo_expiryDate = doc["idTagInfo"]["expiryDate"]; // "2021-12-03T21:34:11.791Z"
                if (strcmp(confMsg["idTagInfo"]["status"], "Accepted") == 0){
                    this->authStatus.authState = STATE_AUTHORIZE_SUCCESS;
                    Payload_AuthorizeRes  AuthorizeRes;
                    Canpacket_ProtocolSend(AuthorizeRes,confMsg);
                    if(!getSessionIdTag()){
                    beginSession(this->authStatus.rfidTag.c_str());
                    }
                }
              }
            },
            [this]() //Abort
            {
              ESP_LOGD(TAG_EMSECC, "authorize Abort! \r\n");
              this->authStatus.authState = STATE_AUTHORIZE_ERROR;
            },
            [this]() //Timeout received
            {
              ESP_LOGD(TAG_EMSECC, "authorize Timeout! \r\n");
              this->authStatus.authState = STATE_AUTHORIZE_TIMEOUT;
            },
            [this](const char *code, const char *description, JsonObject details)
            {
              ESP_LOGD(TAG_EMSECC, "authorize Error ! \r\n");
              this->authStatus.authState = STATE_AUTHORIZE_ERROR;
            }

        );
      }
      else
      {
        ESP_LOGD(TAG_EMSECC, "STATE_AUTHORIZE_UNKNOWN , but no authorization Provided .\r\n");
      }

      break;
    case STATE_AUTHORIZING:
      ESP_LOGD(TAG_EMSECC, "AUTHORIZING , waiting %u!\r\n", loopCount);
      break;
    case STATE_AUTHORIZE_SUCCESS:
      ESP_LOGD(TAG_EMSECC, "Authorized success! (%u)!\r\n", loopCount);

      break;
    case STATE_AUTHORIZE_TIMEOUT:
      ESP_LOGD(TAG_EMSECC, "Authorized STATE_AUTHORIZE_TIMEOUT! (%u)!\r\n", loopCount);
      break;
    case STATE_AUTHORIZE_ERROR:
      ESP_LOGD(TAG_EMSECC, "Authorized STATE_AUTHORIZE_ERROR! (%u)!\r\n", loopCount);
      break;
    default:
      ESP_LOGD(TAG_EMSECC, "authStatus.authState error!(%u)\r\n", (uint32_t)this->authStatus.authState);
      break;
    }
    //this->evRequestsEnergy = false;
    if (
        this->evIsPlugged &&
        this->evRequestsEnergy &&
        (this->authStatus.authState == STATE_AUTHORIZE_SUCCESS))

    {
      //ESP_LOGD(TAG_EMSECC, "----------------------------->SECC_State_Preparing");
      setFsmState(SECC_State_Preparing, NULL);
    }
    else
    {
      ESP_LOGD(TAG_EMSECC, "Waiting : [evIsPlugged:%d] [evRequestsEnergy:%d] [authState:%d - %s - %d]",
               evIsPlugged, evRequestsEnergy, authStatus.authorizationProvided, authStatus.rfidTag.c_str(), authStatus.authState);
    }

    loopCount++;
  }

  void EMSECC::seccPreCharge(void *param)
  {
    (void)param;
    static uint32_t loopCount = 0;
    COMM_ERROR_E retCode = COMM_SUCCESS;

     if(remoteTranscation =true){

     }
    /*
    RequestPayloadEVSE_getEVSEState reqGetEvseState;
    retCode = emEVSE->sendRequest(reqGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Send Request_getEVSEState error:%d while seccPreCharge\r\n", retCode);
      setFsmState(SECC_State_SuspendedSECC, NULL);
      return;
    };

    ResponsePayloadEVSE_getEVSEState resGetEvseState;
    //retCode = emEVSE->receiveResponse(resGetEvseState);
    retCode = receiveRgetEVSEState1(resGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Receive Response_getEVSEState error:%d while seccPreCharge\r\n", retCode);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      return;
    };
    */
    uint8_t CPstatus = Canmodel.getCPstatus();
    switch (CPstatus)
    {
    case CP_Status_INVALID:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP error State:%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    case CP_Status_UNKNOWN:
      ESP_LOGW(TAG_EMSECC, "EVSE CP State Unknown! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      break;
    case CP_Status_A:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State unPlugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = false;
      break;
    case CP_Status_B:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State Plugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      break;
    case CP_Status_C:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State S2! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      this->evRequestsEnergy = true;
      break;
    default:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP State:%s(%d) , unspported!\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      //setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    }
    ESP_LOGD(TAG_EMSECC, "EVSEState [CP=%s(%d)] (RequestsEnergy=%d)",
             //PPStateName[resGetEvseState.statePP].c_str(), resGetEvseState.statePP,
             protocolCPStatus[(CPStatus)CPstatus], CPstatus, this->evRequestsEnergy);

    if (Canmodel.getlockstutus() == locked)
      this->evIsLock = true;
    else
      this->evIsLock = false;

    if (evIsPlugged && evRequestsEnergy && evIsLock)
      setFsmState(SECC_State_Charging, NULL);

    loopCount++;
    sleep(1);
  };

  void EMSECC::seccCharging(void *param)
  {
    (void)param;
    static uint32_t loopCount = 0;
    COMM_ERROR_E retCode = COMM_SUCCESS;

    if (!this->transactionRunning)
    {
      int tid = getTransactionId();
      if (tid < 0)
      {
        ESP_LOGW(TAG_EMSECC, "Start new Transaction");
        startTransaction(this->authStatus.rfidTag.c_str(),
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
              //this->transactionRunning = true;
            });
      }
      else
      {
        this->transactionRunning = true;
        //evRequestsEnergy = true;
        ESP_LOGW(TAG_EMSECC, "TransactionId = %ld , Going on transaction II\n", tid);
      }
    }
    /*
    RequestPayloadEVSE_getEVSEState reqGetEvseState;
    retCode = emEVSE->sendRequest(reqGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Send Request_getEVSEState error:%d while seccPreCharge\r\n", retCode);
      setFsmState(SECC_State_SuspendedSECC, NULL);
      return;
    };

    ResponsePayloadEVSE_getEVSEState resGetEvseState;
    //retCode = emEVSE->receiveResponse(resGetEvseState);
    retCode = receiveRgetEVSEState2(resGetEvseState);
    if (retCode != COMM_SUCCESS)
    {
      ESP_LOGE(TAG_EMSECC, "Receive Response_getEVSEState error:%d while seccPreCharge\r\n", retCode);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      return;
    };*/
    uint8_t CPstatus = Canmodel.getCPstatus();
    switch (CPstatus)
    {
    case CP_Status_INVALID:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP error State:%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    case CP_Status_UNKNOWN:
      ESP_LOGW(TAG_EMSECC, "EVSE CP State Unknown! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      break;
    case CP_Status_A:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State unPlugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = false;
      break;
    case CP_Status_B:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State Plugin! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      break;
    case CP_Status_C:
      ESP_LOGI(TAG_EMSECC, "EVSE CP State S2! :%s(%d)\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      this->evIsPlugged = true;
      this->evRequestsEnergy = true;
      break;
    default:
      ESP_LOGE(TAG_EMSECC, "EVSE Reported CP State:%s(%d) , unspported!\r\n", protocolCPStatus[(CPStatus)CPstatus], CPstatus);
      //setFsmState(SECC_State_SuspendedEVSE, NULL);
      break;
    }
    ESP_LOGD(TAG_EMSECC, "EVSEState [CP=%s(%d)] (RequestsEnergy=%d)",
             //PPStateName[resGetEvseState.statePP].c_str(), resGetEvseState.statePP,
             protocolCPStatus[(CPStatus)CPstatus], CPstatus, this->evRequestsEnergy);

    if (Canmodel.getlockstutus() == locked)
      this->evIsLock = true;
    else
      this->evIsLock = false;

    if (!transactionRunning)
    {
      ESP_LOGD(TAG_EMSECC, "Waiting Transaction confirm ...");
      sleep(2);
      return;
    }

    if (!evIsPlugged || !evRequestsEnergy || !evIsLock){
      setFsmState(SECC_State_Finishing, NULL);
    }
      
    if(Canmodel.StopcharingFlag)
    {
      setFsmState(SECC_State_Finishing, NULL);
      Canmodel.StopcharingFlag = false;
    }  
  };

  void EMSECC::seccFinance(void *param)
  {
    (void)param;
    //ESP_LOGD(TAG_EMSECC, "SECC Finance ...\r\n");//???????????????????????????
    //sleep(3);
    this->FinanceSuccess = false;
    //if(FinanceSuccess){
    endSession();
    this->FinanceSuccess = true;
    //}
   //setFsmState(SECC_State_Waitting, NULL);
  }

  void EMSECC::seccStopCharge(void *param)
  {

    (void)param;

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
    
    setFsmState(SECC_State_Finance, NULL);
    }
  }

  static bool notify = false;
  static String fileName;
  static File file;
  static uint32_t fileSize = 0;
  static uint16_t pages = 0 , currentPage = 0;
  RequestPayloadEVSE_upgrade reqFirmwarePage ;
  void EMSECC::seccMaintaince(void *param)
  {
    (void)param;
    static uint32_t loopCount = 0;
    
    COMM_ERROR_E retCode = COMM_SUCCESS;
    
    if (!loopCount)
      ESP_LOGW(TAG_EMSECC, "SECC Maintaining... \r\n");
    loopCount++;

    fileName = FirmwareProxy::getDownloadFirmwareName();
    if(!fileName.startsWith("/LPC"))
      return;

    if( (!notify) && (FirmwareProxy::proxyDownloadStatusSampler() == DownloadStatus::Downloaded))
    {
      
      if (!USE_FS.exists(fileName.c_str())){
        ESP_LOGE(TAG_EMSECC, "File %s not found!" , fileName.c_str());
        return ;           
      }
      ESP_LOGD(TAG_EMSECC, "Firmware downloaded to file:[%s]" , fileName.c_str() );

      file = USE_FS.open(fileName.c_str(), "r");  //FILE_READ
      if (!file) {
          ESP_LOGE(TAG_EMSECC,"Can't open %s !\r\n", fileName.c_str());
          return ;
      };
      
      fileSize = file.size();
      pages = fileSize/PAGE_SIZE ;
      pages += (fileSize % PAGE_SIZE) ? 1 : 0 ;

      reqFirmwarePage.fileName = fileName ;
      reqFirmwarePage.size = fileSize;
      reqFirmwarePage.pages = pages ; 

      ESP_LOGD(TAG_EMSECC, "Firmware file %s Opened for read , size=%d pages=%d" , fileName.c_str() , fileSize , pages);
      size_t rdSum = 0;
      file.seek(0,SeekSet);
      while(currentPage < pages){
        reqFirmwarePage.currentPage = currentPage;
        memset(reqFirmwarePage.pageData , 0 , PAGE_SIZE);
        size_t rdBytes = file.readBytes(reqFirmwarePage.pageData , PAGE_SIZE);
        rdSum += rdBytes;
        //retCode = emEVSE->sendRequest(reqFirmwarePage);
        retCode = emEVSE->sendRequest_upgrade(reqFirmwarePage);
        ESP_LOGD(TAG_EMSECC,"Send Page:%d (ret=%d) Send Bytes(%d of %d )\r\n",currentPage,retCode , rdBytes , rdSum);
        currentPage++;
        //file.seek(rdBytes,SeekCur);

        //Wait a confirm 
        ResponsePayloadEVSE_upgrade resUpgrade;
        retCode = emEVSE->receiveResponse_upgrade(resUpgrade);
        if( retCode != COMM_SUCCESS){
          ESP_LOGD(TAG_EMSECC,"Receive Response error=%d, upgrade canceld ,CurrentPage=%d !\r\n",retCode , currentPage);
          break;
        }
      } 
      file.close();

      notify = true;
    }


    if((loopCount<200001) && (loopCount % 100000 == 0) && ( FirmwareProxy::proxyInstallationStatusSampler() == InstallationStatus::Installed )){

      ESP_LOGD(TAG_EMSECC,"Frmware installed , Wait reset request."); //reboot !
      ArduinoOcpp::ChargePointStatusService *CPSS = getChargePointStatusService();
      if (CPSS && CPSS->getConnector(0)) {
          CPSS->getConnector(0)->setAvailability(true);
              Serial.println(F("[FirmwareProxy] Set Connector Availability."));
      }
      //It should confirm EVSE is getup!
      sleep(2);
      ESP.restart();
    }
      

  }




  void EMSECC::evseMalfunction(void *param)
  {
    static uint32_t loopCount = 0;
    (void)param;
    //COMM_ERROR_E retCode = COMM_SUCCESS;
    if (!loopCount)
      ESP_LOGE(TAG_EMSECC, "EVSE Malfunction , SECC set state to SECC_State_Unknown 5s after ...\r\n");
    //sleep(5);
    //esp_restart();

    //setFsmState(SECC_State_Unknown, NULL);
    loopCount++;
  }

  void EMSECC::seccMalfunction(void *param)
  {
    //static uint32_t loopCount = 0;
    (void)param;
    //COMM_ERROR_E retCode = COMM_SUCCESS;
    ESP_LOGE(TAG_EMSECC, "SECC Malfunction , reboot 10s after ...\r\n");
    sleep(10);
    esp_restart();
  }

  void EMSECC::loadEvseBehavior(){
    
     setPowerActiveImportSampler([this]() {
        //return (float) (emEVSE->getAmps() * emEVSE->getVoltage());
        return Canmodel.getL1Power()+Canmodel.getL2Power()+Canmodel.getL3Power();
    });

    setEnergyActiveImportSampler([this] () {
        /*float activeImport = 0.f;
        activeImport += (float) emEVSE->getTotalEnergy();
        activeImport += (float) emEVSE->getSessionEnergy();
        return activeImport;*/
        return Canmodel.getTotalElectricity()*1000;//?????????????????? ???KWh?????????Wh
       /* static ulong lastSampled = millis();
                                   static float energyMeter = 0.f;
                                   if (getTransactionId() > 0 && 1)                           // digitalRead(EV_CHARGE_PIN) == EV_CHARGING
                                     energyMeter += ((float)millis() - lastSampled) * 0.003f; //increase by 0.003Wh per ms (~ 10.8kWh per h)
                                   lastSampled = millis();
                                   return energyMeter;*/
    });

    setOnChargingRateLimitChange([this] (float limit) { //limit = maximum charge rate in Watts
      this->secc_setEVSE_PowerLimit(limit);
      //TO-DO For enmax , Notify to EVSE , implement in above function
      //set the SAE J1772 Control Pilot value here

      float amps = limit / 230.f;
      if (amps > 51.f)
      amps = 51.f;

      int pwmVal;
      if (amps < 6.f)
      {
        pwmVal = 256; // = constant +3.3V DC
      }
      else
      {
        pwmVal = (int)(4.26667f * amps);
      }
      //ledcWrite(AMPERAGE_PIN, pwmVal);    //PWM Output?
    });


    setOnUnlockConnector([this](){
     //???lpc???????????? ??????????????????????????????????????????
        Payload_UnlockConnectorReq UnlockConnectorReq;
        Canpacket_ProtocolSend(UnlockConnectorReq);
        //const char* status =  Canmodel.getUnlockconnectorStatus();
      if(Canmodel.getUnlockconnectorStatus() == "Unlocked"){
        return  true;
      }
      
      if(Canmodel.getUnlockconnectorStatus() == "UnlockFailed"){
        return  false;
      }

    });

    setConnectorPluggedSampler([this] () {
       
        return this->evIsPlugged;
    });

    setEvRequestsEnergySampler([this] () {
        
        return this->evRequestsEnergy;
    });

    setConnectorEnergizedSampler([this] () {
        
        return this->evIsLock;
    });

 
    setOnResetReceiveReq([] (JsonObject payload) 
    {   
        const char *type = payload["type"] | "Soft";
        Serial.println(type);
        if (!strcmp(type, "Hard")) {
            Payload_ResetReq ResetReqHard;
            Canpacket_ProtocolSend(ResetReqHard,payload);
            
            sleep(5);
            ESP.restart();//???????????????LPC???????????????
        }

      if(!strcmp(type, "Soft")){
          if (getTransactionId() >= 0)
            {
              stopTransaction();
            }
            Payload_ResetReq ResetReqsoft;
            Canpacket_ProtocolSend(ResetReqsoft,payload);                     
            sleep(5);
            ESP.restart();
          }
    });
      
        

    //???????????????????????????
     setOnRemoteStartTransactionReceiveReq([this](JsonObject payload){
      string authConfirm;
      if (serializeJson(payload, authConfirm))
      {
        ESP_LOGD(TAG_EMSECC, "response confirm: %s \r\n", authConfirm.c_str());
      };
        const char *connectorId =payload["connectorId"];
        const char *idtag = payload["idTag"] ;

        if(getTransactionId()<0){
          this->authStatus.rfidTag.clear();
          this->authStatus.rfidTag.concat(idtag);

          Payload_RemoteStartReq  RemoteStartReq;
          Canpacket_ProtocolSend(RemoteStartReq,payload);//???LPC????????????????????????
          /*if (this->evIsPlugged &&this->evRequestsEnergy )
            {
              ESP_LOGD(TAG_EMSECC, "RemoteStartTransaction-------------->SECC_State_Preparing");
              setFsmState(SECC_State_Preparing, NULL);
            }*/
          setFsmState(SECC_State_Preparing, NULL);
          remoteTranscation = true;
        }
    });
  

      setOnRemoteStopTransactionReceiveReq([this](JsonObject payload){
        if(getTransactionId()>0){
          Payload_RemoteStopReq RemoteStopReq;
          Canpacket_ProtocolSend(RemoteStopReq,payload);//???LPC????????????????????????
          setFsmState(SECC_State_Finishing, NULL);
        }

      }
      );


        //???????????????????????? ???stoptransaction ????????????endSession  ??????unlock ???CS??????????????????
      /*
      addConnectorErrorCodeSampler([this] () {
        if (evse->getEvseState() == OPENEVSE_STATE_OVER_TEMPERATURE) {
            return "HighTemperature";
        }
        return (const char *) NULL;
    });*/
    /*  addConnectorErrorCodeSampler([this] () {
        if (retCode != COMM_SUCCESS) {
            return "EVCommunicationError";
        }
        return (const char *) NULL;
    });//????????????ERROR*/
    //ConnectorLockFailure???ReaderFailure?????????76???
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "GroundFailure") {
          
            return "GroundFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "ConnectorLockFailure") {
          
            return "ConnectorLockFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "EVCommunicationError") {
          
            return "EVCommunicationError";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "HighTemperature") {
          
            return "HighTemperature";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "InternalError") {
          
            return "InternalError";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "LocalListConflict") {
          
            return "LocalListConflict";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "OverCurrentFailure") {
          
            return "OverCurrentFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "OverVoltage") {
          
            return "OverVoltage";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "PowerMeterFailure") {
          
            return "PowerMeterFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "PowerSwitchFailure") {
          
            return "PowerSwitchFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "ReaderFailure") {
          
            return "ReaderFailure";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "UnderVoltage") {
          
            return "UnderVoltage";
        }
        return (const char *) NULL; 
    });
      addConnectorErrorCodeSampler([this] () {
        if (Canmodel.getErrorCode() == "WeakSignal") {
          
            return "WeakSignal";
        }
        return (const char *) NULL; 
    });
      

  }
/*
  void EMSECC::initializeDiagnosticsService(){
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

  }*/