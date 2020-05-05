/*
 * Alarm.cpp
 *
 *  Created on: 13.04.2020
 *      Author: marci
 */

#include "Alarm.h"

Alarm::Alarm() :
    alarmLed(LED3)
{

}

/*
 * sets a particular alarm
 */
void Alarm::set(AlarmID alarmID)
{
    alarmRegister |= (1 << static_cast<int>(alarmID));
    alarmLed = 1;
}

/*
 * display alarm register value
 */
void Alarm::display(CommandVector cv)
{
    printf("Alarms = 0x%08X\r\n", static_cast<unsigned int>(alarmRegister));
}

/*
 * clear alarms
 */
void Alarm::clear(CommandVector cv)
{
    alarmRegister = 0;
    alarmLed = 0;
    printf("Alarms cleared\r\n");
}
