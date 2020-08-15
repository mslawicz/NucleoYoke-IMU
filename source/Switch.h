#ifndef SWITCH_H_
#define SWITCH_H_

#include <mbed.h>

enum class SwitchType
{
    Connector,
    RotaryEncoder
};

/*
Class of various types of switches (pushbutton, toggle switch, rotary encoder)
*/
class Switch
{
public:
    Switch(SwitchType switchType, PinName levelPin, float debounceTimeout = 0.01f, PinName directionPin = NC);
    //XXX void setCallback(Callback<void(uint8_t)> cb) { userCb = cb; }
private:
    void handler(void);     // it must be called on every access
    SwitchType switchType;
    DigitalIn level;
    float debounceTimeout;
    DigitalIn direction;
    bool isStable{true};
    int currentLevel{1};
    int previousLevel{1};
    Timer debounceTimer;
    bool changedToOne{false};
    bool changedToZero{false};
    //XXX Callback<void(uint8_t)> userCb{nullptr};    // callback function called with argument=0 (left turn) or 1 (right turn)
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