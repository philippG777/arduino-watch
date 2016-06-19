class Btn
{
  public:
    Btn(uint8_t p, unsigned int t = 0)
    {
      _pin = p;
      _nextDebounceTime = 0;
      _pulseLogic = _lastStateLogic = false;
      _pulseDelay = t;
      _repeatPulseTime = 0;
    }
    bool state()
    {
      return _stateLogic;
    }
    bool pulse()
    {
      return _pulseLogic;
    }
    void debounce(unsigned long nowMs)
    {
      _pulseLogic = false;
      if (nowMs > _nextDebounceTime)
      {
        _newState = !digitalRead(_pin); //pull-up
        if (_oldState != _newState)
        {
          _nextDebounceTime = nowMs + debounceDelay;
        }
        else
        {
          _stateLogic = _newState;
          if (_stateLogic && (!_lastStateLogic || (_pulseDelay != 0) && (nowMs > _repeatPulseTime)))
          {
            _pulseLogic = true;
            _repeatPulseTime = nowMs + _pulseDelay;
          }
          _lastStateLogic = _stateLogic;
        }
        _oldState = _newState;
      }
    }
  private:
    // logical States
    bool _stateLogic;
    bool _pulseLogic;
    bool _lastStateLogic;
    unsigned long _repeatPulseTime;
    unsigned int _pulseDelay;

    // Hardware
    bool _newState;
    bool _oldState;
    uint8_t _pin;
    unsigned long _nextDebounceTime;


};

