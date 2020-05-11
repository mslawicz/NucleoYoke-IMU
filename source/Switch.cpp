#include "Switch.h"

Switch::Switch(PinName dataPin, PinName clkPin, EventQueue& eventQueue) :
    data(dataPin, PullUp),
    clk(clkPin, PullUp),
    eventQueue(eventQueue)
{
    clk.fall(callback(this, &Switch::onClockFallInterrupt));
    clk.rise(callback(this, &Switch::onClockRiseInterrupt));
}

void Switch::onClockFallInterrupt(void)
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
    clockDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), DebounceTimeout);
}

void Switch::onClockRiseInterrupt(void)
{
    clockDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), DebounceTimeout);
}

void Switch::onDebounceTimeoutCb(void)
{
    if(clk.read() == 1)
    {
        stableHigh = true;
    }
}