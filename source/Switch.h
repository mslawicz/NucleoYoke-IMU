#ifndef SWITCH_H_
#define SWITCH_H_

#include <mbed.h>

enum class SwitchType
{
    Pushbutton,
    ToggleSwitch,
    BinaryEncoder
};

class Switch
{
public:
    Switch(SwitchType switchType, PinName levelPin, EventQueue& eventQueue, float debounceTimeout = 0.01f, PinName directionPin = NC);
    void setCallback(Callback<void(uint8_t)> cb) { userCb = cb; }
private:
    void onLevelFallInterrupt(void);
    void onLevelRiseInterrupt(void);
    void onDebounceTimeoutCb(void);
    SwitchType switchType;
    InterruptIn level;
    EventQueue& eventQueue;
    float debounceTimeout;
    DigitalIn direction;
    Timeout levelDebounceTimeout;
    bool stableHigh{true};
    bool stableLow{false};
    Callback<void(uint8_t)> userCb{nullptr};    // callback function called with argument=0 (left turn) or 1 (right turn)
};

#endif /* ENCSWITCH_H_ODER_H_ */