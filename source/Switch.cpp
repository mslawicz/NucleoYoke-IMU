#include "Switch.h"

Switch::Switch(SwitchType switchType, PinName levelPin, EventQueue& eventQueue, float debounceTimeout, PinName directionPin) :
    switchType(switchType),
    level(levelPin, PullUp),
    eventQueue(eventQueue),
    debounceTimeout(debounceTimeout),
    direction(directionPin, PullUp)
{
    level.fall(callback(this, &Switch::onLevelFallInterrupt));
    level.rise(callback(this, &Switch::onLevelRiseInterrupt));
}

void Switch::onLevelFallInterrupt(void)
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
    levelDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), debounceTimeout);
}

void Switch::onLevelRiseInterrupt(void)
{
    levelDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), debounceTimeout);
}

void Switch::onDebounceTimeoutCb(void)
{
    if(level.read() == 1)
    {
        stableHigh = true;
    }
    else
    {
        stableLow = true;
    }
}