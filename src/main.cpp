//#include <Variants.h>
#include <Arduino.h>

#include <unistd.h>
#include <esp_log.h>

#define NETWORK_WIFI        1
#define NETWORK_ETHERNET    2
#define NETWORK_CELLULAR    3
#define NETWORK_TYPE        NETWORK_WIFI

#if( NETWORK_TYPE == NETWORK_ETHERNET )
  #include <ETH.h>
  #define ETH_ADDR        1
  #define ETH_POWER_PIN  -1
  #define ETH_MDC_PIN    23
  #define ETH_MDIO_PIN   18
  #define ETH_TYPE       ETH_PHY_LAN8720
  #define ETH_CLK_MODE   ETH_CLOCK_GPIO17_OUT
#elif( NETWORK_TYPE == NETWORK_WIFI )
  #include <WiFi.h>
#elif( NETWORK_TYPE == NETWORK_CELLULAR )
  #include "cellular.hpp"
  #include <TinyGsmClient.h>
  #include <TinyGsmCommon.h>
#else
  #error NETWORK_TYPE define error
#endif

bool networkReady =  false;
#include "time.h"
#include "lwip/apps/sntp.h"

//ArduinoOcpp modules
#include <ArduinoJson.h>
#include <core/NewOCPP.h>
#include <ArduinoOcpp/Core/OcppEngine.h>
#include <ArduinoOcpp/Core/Configuration.h>

#include "emChargePoint.hpp"
#include "emSECC.hpp"
#include "Common.hpp"
#include "SECC_SPI.hpp"
#include "driver/spi_master.h"
#include "can/SECCCan.h"
//HardwareSerial ESP_Uart1(1);
/*
#include "task/ocpp.h"
#include "task/EVSEModel.h"
#include "MongooseCore.h"
#include "task/event_log.h"
#include "DiagService.h"
#include "task/root_ca.h"*/
#include "LittleFS.h"

#include "RS485/enmMeter.h"

#define TAG  "TAG:"
#define SPI_SCK 14
#define SPI_MISO 12
#define SPI_MOSI 13
#define SPI_CS 15
//static const int spiClk = 20000000;
SECC_SPIClass * hspi = new SECC_SPIClass(HSPI);
//EVSEModelCan Canmodel;
//SoftwareSerial IECinterface(35, 33);
int DE_RE=32;
static void hw_init()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    /*//HardwareSerial ESP_Uart1(1);
    //begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
    pinMode(UART1_RX_PIN,INPUT_PULLUP);
    ESP_Uart1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
    
      ESP_Uart1.println("[Bootting...]\r\n"); */
    pinMode(34,INPUT);
    pinMode(SPI_CS,OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    hspi->begin(SPI_SCK ,SPI_MISO ,SPI_MOSI ,SPI_CS);

  //IECinterface.begin(115200);
  pinMode(DE_RE,OUTPUT);
  digitalWrite(DE_RE,LOW);  
}

static void log_setup()
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    log_v("Verbose");
    log_d("Debug");
    log_i("Info");
    log_w("Warning");
    log_e("Error");

    ESP_LOGV("SYS:", "VERBOSE");
    ESP_LOGD("SYS:", "DEBUG");
    ESP_LOGI("SYS:", "INFO");
    ESP_LOGW("SYS:", "WARNING");
    ESP_LOGE("SYS:", "ERROR");

    esp_log_level_set(TAG_EMSECC, ESP_LOG_DEBUG);
    esp_log_level_set(TAG_INTF, ESP_LOG_ERROR);
    esp_log_level_set(TAG_PROT, ESP_LOG_ERROR);
}

#if( NETWORK_TYPE == NETWORK_ETHERNET )
static void lan_setup(){
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); //??????ETH
    Serial.print(F("[main] Wait for Network...: "));
    while (!networkReady)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.print(F("LAN connected!\r\n"));
}
void ethEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START: //??????ETH??????
    Serial.println("ETH Started");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED: //????????????
    Serial.println("ETH Connected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP: //??????IP
    Serial.println("ETH GOT IP");
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
        Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    networkReady=true;
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED: //????????????
    Serial.println("ETH Disconnected");
    break;
  case SYSTEM_EVENT_ETH_STOP: //??????
    Serial.println("ETH Stopped");
    break;
  default:
    break;
  }
}

#elif( NETWORK_TYPE == NETWORK_WIFI )
static void wifi_setup()
{
    Serial.println("Bootload version 22.1.13 build 1031\r\n");

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
        Serial.flush();
        Serial.print(".");
        delay(300);
    }

    WiFi.begin(STASSID, STAPSK);
    WiFi.setSleep(0);//??????modem sleep??????????????????

    Serial.print(F("[main] Wait for Network...: "));
    while (!WiFi.isConnected())
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.print(F("WIFI connected!\r\n"));

}

#elif( NETWORK_TYPE == NETWORK_CELLULAR )

#endif



static void esp_setup_sntp(void)
{
    const char* ntpServer = "pool.ntp.org";     //cn.pool.ntp.org
    const long  gmtOffset_sec = 0;
    const int   daylightOffset_sec = 3600*8;    //TZ East+8
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

static void esp_initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "cn.pool.ntp.org");
    sntp_setservername(2, "ntp1.aliyun.com");

    sntp_init();

}

static void ocpp_config(){
  auto AuthorizeRemoteTxRequests = declareConfiguration("AuthorizeRemoteTxRequests", true, CONFIGURATION_VOLATILE, false, true, false, false);//?????????true ??????
  auto ClockAlignedDataInterval = declareConfiguration("ClockAlignedDataInterval", 900);//?????????false
  //auto ConnectorPhaseRotation = declareConfiguration("ConnectorPhaseRotation", "0.RST");//??????CSL????????????????????????:0.RST, 1.RST, 2.RTS
  auto GetConfigurationMaxKeys = declareConfiguration("GetConfigurationMaxKeys", 20, CONFIGURATION_VOLATILE, false, true, false, false);
  auto LocalAuthorizeOffline = declareConfiguration("LocalAuthorizeOffline", false);
  auto LocalPreAuthorize = declareConfiguration("LocalPreAuthorize", false);
  //auto MeterValuesAlignedData = declareConfiguration("MeterValuesAlignedData", false);//??????CSL????????????
  auto MeterValuesSampledData = declareConfiguration("MeterValuesSampledData", "Energy.Active.Import.Register");//??????CSL????????????
  auto ResetRetries = declareConfiguration("ResetRetries", 3);

}

//dia *da; 
//EventLog eventLog;
//OcppTask ocppMD = OcppTask();
//EVSEModel *evse;
EMSECC *emSecc ;
RS485IF MeterIF(35,33);
long previousTime=0;
long interval=15000;
void setup() {
    //pinMode(GPIO_NUM_0,PULLUP);
    //USE_FS.begin(true);
    //File eventFile = USE_FS.open("/eventlog","r");
    pinMode(SPI_CS,OUTPUT);
    hw_init();
    log_setup();

#if( NETWORK_TYPE == NETWORK_ETHERNET )
    WiFi.onEvent(ethEvent); //????????????
    lan_setup();
    //esp_setup_sntp();
    esp_initialize_sntp();
    sntp_enabled();
    struct tm sysTimeinfo = { 0 };
    esp_sync_sntp(sysTimeinfo, 5);
    esp_set_systime(sysTimeinfo);
#elif( NETWORK_TYPE == NETWORK_WIFI )
    wifi_setup();
    //esp_setup_sntp();
    esp_initialize_sntp();
    sntp_enabled();
    struct tm sysTimeinfo = { 0 };
    esp_sync_sntp(sysTimeinfo, 5);
    esp_set_systime(sysTimeinfo);    
#elif( NETWORK_TYPE == NETWORK_CELLULAR )   
    cellular_setup();
    cellular_attach();
#endif

    //eventLog.begin();
    //evse = new EVSEModel(hspi);
    //ocppMD.begin(evse,eventLog);

    //Mongoose.begin();
    //Mongoose.setRootCa(root_ca);
    emSecc = new EMSECC(hspi);
    //da =new dia(eventLog);
    
    //auto connectionTimeOut = declareConfiguration<int>("ConnectionTimeOut", 60, CONFIGURATION_FN, true, true, true, false);
    //intervalConf = declareConfiguration<int>("HeartbeatInterval", 86400);
    //MeterValueSampleInterval = declareConfiguration("MeterValueSampleInterval", 60);?????????false
    //MeterValuesSampledDataMaxLength = declareConfiguration("MeterValuesSampledDataMaxLength", 4, CONFIGURATION_VOLATILE, false, true, false, false);?????????true
    //ocpp_config();
    uint64_t mac = ESP.getEfuseMac();
    String cpSerialNum = String((unsigned long)mac , 16);
    String cpModel = String(CP_Model);
    String cpVendor = String(CP_Vendor);    
    String csUrl =  String(OCPP_URL)+cpVendor+'_'+cpModel+'_'+cpSerialNum ;
    
    String ocppHost = OCPP_HOST;
    const char *ocpphost = ocppHost.c_str();
    OCPP_initialize(ocpphost, OCPP_PORT, csUrl.c_str());
    


    
    string ocpptimeString ;
    esp_get_Ocpptime( ocpptimeString );
    ArduinoOcpp::OcppTime *ocppTime = new ArduinoOcpp::OcppTime(ArduinoOcpp::Clocks::DEFAULT_CLOCK);
    ocppTime->setOcppTime(ocpptimeString.c_str());
    sntp_stop();


    /* bootNotification(CP_Model, CP_Vendor, //"enrgmax", "EU-DC-em21-evse",
                     [](JsonObject confMsg)
                     {
                       //This callback is executed when the .conf() response from the central system arrives
                       ESP_LOGD(TAG_EMSECC, "BootNotification was answered. Central System clock: %s", confMsg["currentTime"].as<const char *>()); //as<string>()??????
                       //this->evseIsBooted = true;
                       //esp_set_OcppTime(confMsg["currentTime"].as<const char *>());
                     }); */
}

void loop() {
  Canmodel.loop();
  emSecc->secc_loop();
  OCPP_loop(); 
  /*
  unsigned long currentTime=millis();
	if(currentTime - previousTime > interval){
  	previousTime=currentTime;
    MeterIF.loop();
  }
*/
}