#include "Encoder.h"

Encoder::Encoder(PinName dataPin, PinName clkPin, EventQueue& eventQueue) :
    data(dataPin, PullUp),
    clk(clkPin, PullUp),
    eventQueue(eventQueue)
{
    clk.fall(callback(this, &Encoder::onClockFallInterrupt));
    clk.rise(callback(this, &Encoder::onClockRiseInterrupt));
}

void Encoder::onClockFallInterrupt(void)
{
    if(stableHigh)
    {
        // execute user callback
        if(userCb)
        {
            eventQueue.call(userCb, data.read());
        }
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