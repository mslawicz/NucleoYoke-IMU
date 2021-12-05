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
    Switch(SwitchType switchType, PinName levelPin, EventQueue& eventQueue, float debounceTimeout = 0.01F, PinName directionPin = NC);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
    void setCallback(Callback<void(uint8_t)> cb) { userCb = cb; }
private:
    void onLevelFallInterrupt();
    void onLevelRiseInterrupt();
    void onDebounceTimeoutCb();
    SwitchType switchType;
    InterruptIn level;
    EventQueue& eventQueue;
    float debounceTimeout;
    DigitalIn direction;
    Timeout levelDebounceTimeout;
    bool stableHigh{true};
    bool stableLow{false};
    Callback<void(uint8_t)> userCb{nullptr};    // callback function called with argument=0 (left turn) or 1 (right turn)
    static constexpr uint32_t UsInSec = 1000000U; 
};

/*
class of the HAT switch
*/
class Hat
{
public:
    Hat(PinName northPin, PinName eastPin, PinName southPin, PinName westPin);
    uint8_t getPosition();
private:
    BusIn switchBus;
};

#endif /* ENCSWITCH_H_ODER_H_ */