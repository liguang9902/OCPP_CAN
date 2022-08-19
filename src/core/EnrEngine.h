#ifndef ENRENGINE_H
#define ENRENGINE_H

#include <ArduinoOcpp/Core/OcppConnection.h>
#include <ArduinoOcpp/Core/OcppTime.h>
#include <memory>
#include "ArduinoOcpp/Core/OcppEngine.h"
#include "ArduinoOcpp/Core/OcppModel.h"


namespace ArduinoOcpp {

class OcppSocket;
class OcppModel;

class EnrOcppEngine: public OcppEngine {


public:
   EnrOcppEngine(OcppSocket& ocppSocket, const OcppClock& system_clock);

};



} //end namespace ArduinoOcpp

#endif