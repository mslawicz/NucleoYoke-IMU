/*
 * Alarm.h
 *
 *  Created on: 13.04.2020
 *      Author: marci
 */

#ifndef ALARM_H_
#define ALARM_H_

#include "Console.h"
#include "mbed.h"

enum class AlarmID
{
    I2CWrite,
    I2CWriteBeforeRead,
    I2CReadAfterWrite
};

class Alarm
{
public:
    Alarm(Alarm const&) = delete;       // do not allow copy constructor of a singleton
    void operator=(Alarm const&) = delete;
    static Alarm& getInstance();
    void set(AlarmID alarmId);
    void display(CommandVector cv);
    void clear(CommandVector cv);
private:
    Alarm();
    uint32_t alarmRegister{0};
    DigitalOut alarmLed;
};

#endif /* ALARM_H_ */
