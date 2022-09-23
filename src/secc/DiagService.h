#ifndef DIAGSERVICE_H
#define DIAGSERVICE_H

#include "core/NewOCPP.h"
#include "task/event_log.h"
#include "MongooseHttpClient.h"
class dia{
    private:
    EventLog *eventLog;
    bool diagSuccess = false, diagFailure = false;
    MongooseHttpClient diagClient = MongooseHttpClient();
    public:
    dia(EventLog &eventLog);
    void    begin();
};

#endif