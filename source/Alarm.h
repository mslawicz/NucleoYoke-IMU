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
    I2CReadAfterWrite,
    NoImuInterrupt
};

class Alarm
{
public:
    Alarm(Alarm const&) = delete;       // do not allow copy constructor of a singleton
    void operator=(Alarm const&) = delete;
    Alarm(Alarm&&) = delete;
    void operator=(Alarm&&) = delete;
    static Alarm& getInstance();
    void set(AlarmID alarmId);
    void display(CommandVector& cv) const;
    void clear(CommandVector& cv);
    void displayOnScreen() const;
private:
    Alarm();
    ~Alarm() = default;
    void clearFromMenu();
    uint32_t alarmRegister{0};
    DigitalOut alarmLed;
};

#endif /* ALARM_H_ */
