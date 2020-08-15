#include "Switch.h"

Switch::Switch(SwitchType switchType, PinName levelPin, float debounceTimeout, PinName directionPin) :
    switchType(switchType),
    level(levelPin, PullUp),
    debounceTimeout(debounceTimeout),
    direction(directionPin, PullUp)
{
    debounceTimer.start();
}

void Switch::handler(void)
{
    int newLevel = level.read();
    if(isStable)
    {
        if(newLevel != currentLevel)
        {
            // signal was stable and now is changed
            currentLevel = newLevel;
            isStable = false;
            debounceTimer.reset();
            //printf("XXX new level %d\n", currentLevel);

            if(switchType == SwitchType::RotaryEncoder)
            {
                if(currentLevel == 0)
                {
                    // sample direction on clock falling edge
                    if(direction.read() == 0)
                    {
                        changedToZero = true;
                    }
                    else
                    {
                        changedToOne = true;
                    }
                }
            }
            else
            {
                // pushbutton and toggle switch
                if(currentLevel)
                {
                    changedToOne = true;
                    //printf("XXX changed to 1\n");
                }
                else
                {
                    changedToZero = true;
                    //printf("XXX changed to 0\n");
                }
            }
        }
    }
    else
    {
        // signal is not stable
        if(chrono::duration<float>(debounceTimer.elapsed_time().count()) > chrono::duration<float>(debounceTimeout))
        {
            // signal is stable long enough
            isStable = true;
            //printf("XXX timeout reached\n");
        }
        else if(newLevel != previousLevel)
        {
            // signal is still bouncing
            //printf("XXX debouncing %d  %f\n", newLevel, chrono::duration<float>(debounceTimer.elapsed_time().count()));
            debounceTimer.reset();
        }
    }

    previousLevel = newLevel;
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
    case 0x0E:
        hatPosition = 1;
        break;
    case 0x0C:
        hatPosition = 2;
        break;
    case 0x0D:
        hatPosition = 3;
        break;
    case 0x09:
        hatPosition = 4;
        break;
    case 0x0B:
        hatPosition = 5;
        break;
    case 0x03:
        hatPosition = 6;
        break;
    case 0x07:
        hatPosition = 7;
        break;
    case 0x06:
        hatPosition = 8;
        break;
    default:
        hatPosition = 0;
        break;
    }
    return hatPosition;
}