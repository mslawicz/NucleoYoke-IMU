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

Hat::Hat(PinName northPin, PinName eastPin, PinName southPin, PinName westPin) :
    switchBus(northPin, eastPin, southPin, westPin)
{
    switchBus.mode(PullUp);
}

/*
get position of the HAT switch
0-neutral
1-N
2-NE
3-E
4-SE
5-S
6-SW
7-W
8-NW
*/
uint8_t Hat::getPosition(void)
{
    uint8_t busValue = switchBus.read() & 0x0F;
    uint8_t hatPosition;
    switch(busValue)
    {
    case 1:
        hatPosition = 1;
        break;
    case 3:
        hatPosition = 2;
        break;
    case 2:
        hatPosition = 3;
        break;
    case 6:
        hatPosition = 4;
        break;
    case 4:
        hatPosition = 5;
        break;
    case 12:
        hatPosition = 6;
        break;
    case 8:
        hatPosition = 7;
        break;
    case 9:
        hatPosition = 8;
        break;
    default:
        hatPosition = 0;
        break;
    }
    return hatPosition;
}