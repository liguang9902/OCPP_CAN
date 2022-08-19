
#include "core/EnrEngine.h"
#include <ArduinoOcpp/Core/OcppOperation.h>
#include <ArduinoOcpp/Core/OcppSocket.h>
#include <ArduinoOcpp/Core/OcppModel.h>
#include "ArduinoOcpp/Core/OcppEngine.h"

using namespace ArduinoOcpp;



EnrOcppEngine::EnrOcppEngine(OcppSocket& ocppSocket, const OcppClock& system_clock)
        : OcppEngine(ocppSocket,system_clock) {
    
}