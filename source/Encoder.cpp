#include "Encoder.h"

// XXX object test
Encoder testEncoder(PG_2, PG_3);

Encoder::Encoder(PinName dataPin, PinName clkPin) :
    data(dataPin, PullUp),
    clk(clkPin, PullUp)
{
    clk.fall(callback(this, &Encoder::onClockFallInterrupt));
    clk.rise(callback(this, &Encoder::onClockRiseInterrupt));
}

void Encoder::onClockFallInterrupt(void)
{
    if(stableHigh)
    {
        // execute client callback here
        stableHigh = false;
    }
    clockDebounceTimeout.attach(callback(this, &Encoder::onDebounceTimeoutCb), DebounceTimeout);
}

void Encoder::onClockRiseInterrupt(void)
{
    clockDebounceTimeout.attach(callback(this, &Encoder::onDebounceTimeoutCb), DebounceTimeout);
}

void Encoder::onDebounceTimeoutCb(void)
{
    if(clk.read() == 1)
    {
        stableHigh = true;
    }
}