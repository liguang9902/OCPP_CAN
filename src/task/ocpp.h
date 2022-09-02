#include "core/NewOCPP.h"
#include "task/EVSEModel.h"

#include "MicroTasks.h"
#include "MongooseHttpClient.h"
#include "task/event_log.h"
    class OcppTask :public MicroTasks::Task
    {
    private:
        EVSEModel *evse;
        EventLog *eventLog;


        String ocpp_idTag ;
        bool transactionInitialized = false; 
        bool transactionRunning = false;
        float charging_limit = -1.f; //in Watts. chargingLimit < 0 means that there is no Smart Charging (and no restrictions )
        int ocppTxIdDisplay {-1};
        bool ocppSessionDisplay {false};

        bool vehicleConnected = false;

        std::function<bool(const String& idTag)> onIdTagInput {nullptr};

        bool resetTriggered = false;
        bool resetHard = false; //default to soft reset
        ulong resetTime;

        bool OcppInitialized = false;
        void initializeArduinoOcpp();
        void loadEvseBehavior();

        MongooseHttpClient diagClient = MongooseHttpClient();
        bool diagSuccess = false, diagFailure = false;
        void initializeDiagnosticsService();

        bool updateSuccess = false, updateFailure = false;
        void initializeFwService();

        static OcppTask *instance;

        //helper functions
        static bool idTagIsAccepted(JsonObject payload);
        void updateEvseClaim();

         ulong updateEvseClaimLast {0};
    public:
        OcppTask();
        ~OcppTask();
        void begin(EVSEModel *evse,EventLog &eventLog);

        void setup();
        unsigned long loop(MicroTasks::WakeReason reason);
    };
    
    