#include "Switch.h"

Switch::Switch(PinName statePin, EventQueue& eventQueue, float debounceTimeout, PinName directionPin) :
    state(statePin, PullUp),
    eventQueue(eventQueue),
    debounceTimeout(debounceTimeout),
    direction(directionPin, PullUp)
{
    state.fall(callback(this, &Switch::onClockFallInterrupt));
    state.rise(callback(this, &Switch::onClockRiseInterrupt));
}

void Switch::onClockFallInterrupt(void)
{
    if(stableHigh)
    {
        // execute user callback
        if(userCb)
        {
            eventQueue.call(userCb, direction.read());
        }
        stableHigh = false;
    }
    clockDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), debounceTimeout);
}

void Switch::onClockRiseInterrupt(void)
{
    clockDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), debounceTimeout);
}

void Switch::onDebounceTimeoutCb(void)
{
    if(state.read() == 1)
    {
        stableHigh = true;
    }
}