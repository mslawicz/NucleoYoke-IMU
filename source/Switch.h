#ifndef SWITCH_H_
#define SWITCH_H_

#include <mbed.h>

class Switch
{
public:
    Switch(PinName statePin, EventQueue& eventQueue, float debounceTimeout = 0.01f, PinName directionPin = NC);
    void setCallback(Callback<void(uint8_t)> cb) { userCb = cb; }
private:
    void onClockFallInterrupt(void);
    void onClockRiseInterrupt(void);
    void onDebounceTimeoutCb(void);
    InterruptIn state;
    EventQueue& eventQueue;
    float debounceTimeout;
    DigitalIn direction;
    Timeout clockDebounceTimeout;
    bool stableHigh{true};
    Callback<void(uint8_t)> userCb{nullptr};    // callback function called with argument=0 (left turn) or 1 (right turn)
};

#endif /* ENCSWITCH_H_ODER_H_ */