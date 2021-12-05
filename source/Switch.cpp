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

void Switch::onLevelFallInterrupt()
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
    levelDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), std::chrono::microseconds(static_cast<uint64_t>(debounceTimeout * UsInSec)));
}

void Switch::onLevelRiseInterrupt()
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
    levelDebounceTimeout.attach(callback(this, &Switch::onDebounceTimeoutCb), std::chrono::microseconds(static_cast<uint64_t>(debounceTimeout * UsInSec)));
}

void Switch::onDebounceTimeoutCb()
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
uint8_t Hat::getPosition()
{
    auto busValue = static_cast<uint8_t>(switchBus.read() & 0x0F);       //NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
    uint8_t hatPosition{0};
    switch(busValue)
    {
    case 0x0E:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 1;
        break;
    case 0x0C:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 2;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x0D:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 3;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x09:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 4;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x0B:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 5;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x03:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 6;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x07:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 7;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    case 0x06:      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        hatPosition = 8;        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
        break;
    default:
        hatPosition = 0;
        break;
    }
    return hatPosition;
}