#ifndef ENCODER_H_
#define ENCODER_H_

#include <mbed.h>

class Encoder
{
public:
    Encoder(PinName dataPin, PinName clkPin);
private:
    void onClockFallInterrupt(void);
    void onClockRiseInterrupt(void);
    void onDebounceTimeoutCb(void);
    DigitalIn data;
    InterruptIn clk;
    Timeout clockDebounceTimeout;
    const float DebounceTimeout = 0.01f;
    bool stableHigh{true};
};

#endif /* ENCODER_H_ */