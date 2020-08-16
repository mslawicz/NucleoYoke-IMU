#ifndef SWITCH_H_
#define SWITCH_H_

#include <mbed.h>

enum class SwitchType
{
    Pushbutton,
    ToggleSwitch,
    RotaryEncoder
};

/*
Class of various types of switches (pushbutton, toggle switch, rotary encoder)
Caution! Do not use the same levelPin number for several objects
*/
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

/*
class of the HAT switch
*/
class Hat
{
public:
    Hat(PinName northPin, PinName eastPin, PinName southPin, PinName westPin);
    uint8_t getPosition(void);
private:
    BusIn switchBus;
};

#endif /* ENCSWITCH_H_ODER_H_ */