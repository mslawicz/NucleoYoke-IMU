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
        // execute user callback on falling edge
        if(userCb)
        {
            switch(switchType)
            {
                case SwitchType::RotaryEncoder:
                    eventQueue.call(userCb, direction.read());
                    break;
                case SwitchType::Pushbutton:
                case SwitchType::ToggleSwitch:
                    eventQueue.call(userCb, 0);
                    break;
                default:
                    break;
            }
        }
        stableHigh = false;
    }
    levelDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), debounceTimeout);
}

void Switch::onLevelRiseInterrupt(void)
{
    if(stableLow)
    {
        // execute user callback on rising edge
        if(userCb)
        {
            switch(switchType)
            {
                case SwitchType::ToggleSwitch:
                    eventQueue.call(userCb, 1);
                    break;
                case SwitchType::Pushbutton:
                case SwitchType::RotaryEncoder:
                default:
                    break;
            }
        }
        stableLow = false;
    }
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