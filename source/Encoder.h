#ifndef ENCODER_H_
#define ENCODER_H_

#include <mbed.h>

class Encoder
{
public:
    Encoder(PinName dataPin, PinName clkPin, EventQueue& eventQueue);
    void setCallback(Callback<void(uint8_t)> cb) { userCb = cb; }
private:
    void onClockFallInterrupt(void);
    void onClockRiseInterrupt(void);
    void onDebounceTimeoutCb(void);
    DigitalIn data;
    InterruptIn clk;
    EventQueue& eventQueue;
    Timeout clockDebounceTimeout;
    const float DebounceTimeout = 0.01f;
    bool stableHigh{true};
    Callback<void(uint8_t)> userCb{nullptr};    // callback function called with argument=0 (left turn) or 1 (right turn)
};

#endif /* ENCODER_H_ */