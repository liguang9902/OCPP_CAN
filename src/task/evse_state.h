
#ifndef _OPENEVSE_EVSE_STATE_H
#define _OPENEVSE_EVSE_STATE_H

#include <Arduino.h>

class EvseState
{
  public:
    enum Value : uint8_t {
      None,
      Active,
      Disabled
    };

  EvseState() = default;
  constexpr EvseState(Value value) : _value(value) { }

  bool fromString(const char *value)
  {
    // Cheat a bit and just check the first char
    if('a' == value[0] || 'd' == value[0]) {
      _value = 'a' == value[0] ? EvseState::Active : EvseState::Disabled;
      return true;
    }
    return false;
  }

  const char *toString()
  {
    return EvseState::Active == _value ? "active" :
           EvseState::Disabled == _value ? "disabled" :
           EvseState::None == _value ? "none" :
           "unknown";
  }

  operator Value() const { return _value; }
  explicit operator bool() = delete;        // Prevent usage: if(state)
  EvseState operator= (const Value val) {
    _value = val;
    return *this;
  }

  private:
    Value _value;
};

class EvseProperties 
{
  private:
    EvseState _state;
    uint32_t _charge_current;
    uint32_t _max_current;
    uint32_t _energy_limit;
    uint32_t _time_limit;
    bool _auto_release;
  public:
    EvseProperties()= default;
    EvseProperties(EvseState state);

    void clear();

    // Get/set the EVSE state, either active or disabled
    EvseState getState() {
      return _state;
    }
    void setState(EvseState state) {
      _state = state;
    }

    // Get/set charge current,
    uint32_t getChargeCurrent() {
      return _charge_current;
    }
    void setChargeCurrent(uint32_t charge_current) {
      _charge_current = charge_current;
    }

    // Get/set the max current, overides limits the charge current (irrespective of priority) but
    // does not override the configured max charge current of the hardware.
    uint32_t getMaxCurrent() {
      return _max_current;
    }
    void setMaxCurrent(uint32_t max_current) {
      _max_current = max_current;
    }

    // Get/set the energy max to transfer for this charge session/client, after which the default
    // session state will be set to EvseState::Disabled and the client automatically released.
    uint32_t getEnergyLimit() {
      return _energy_limit;
    }
    void setEnergyLimit(uint32_t energy_limit) {
      _energy_limit = energy_limit;
    }

    // Get/set the time to stop the charge session/client, after which the default session state
    // will be set to EvseState::Disabled and the client automatically released.
    uint32_t getTimeLimit() {
      return _time_limit;
    }
    void setTimeLimit(uint32_t time_limit) {
      _time_limit = time_limit;
    }

    // Get/set the client auto release state. With the client auto release enabled the client claim
    // will automatically be released at the end of the charging session.
    bool isAutoRelease() {
      return _auto_release;
    }
    void setAutoRelease(bool auto_release) {
      _auto_release = auto_release;
    }

    EvseProperties & operator = (EvseProperties &rhs);
    EvseProperties & operator = (EvseState &rhs) {
      _state = rhs;
      return *this;
    }

    bool equals(EvseProperties &rhs) {
      return this->_state == rhs._state &&
             this->_charge_current == rhs._charge_current &&
             this->_max_current == rhs._max_current &&
             this->_energy_limit == rhs._energy_limit &&
             this->_time_limit == rhs._time_limit &&
             this->_auto_release == rhs._auto_release;

    }
    bool equals(EvseState &rhs) {
      return this->_state == rhs;
    }

    bool operator == (EvseProperties &rhs) {
      return this->equals(rhs);
    }
    bool operator == (EvseState &rhs) {
      return this->equals(rhs);
    }

    bool operator != (EvseProperties &rhs) {
      return !equals(rhs);
    }
    bool operator != (EvseState &rhs) {
      return !equals(rhs);
    }

};

#endif // _OPENEVSE_EVSE_STATE_H
