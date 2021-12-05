/*
 * Alarm.cpp
 *
 *  Created on: 13.04.2020
 *      Author: marci
 */

#include "Alarm.h"
#include "Display.h"
#include "Logger.h"
#include "Menu.h"
#include <iostream>

Alarm::Alarm() :
    alarmLed(LED3, 0)
{
    Console::getInstance().registerCommand("da", "display alarms", callback(this, &Alarm::display));
    Console::getInstance().registerCommand("ca", "clear alarms", callback(this, &Alarm::clear));
    Menu::getInstance().addItem("clear alarms", callback(this, &Alarm::clearFromMenu));
}

Alarm& Alarm::getInstance()
{
    static Alarm instance;  // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

/*
 * sets a particular alarm
 */
void Alarm::set(AlarmID alarmID)
{
    alarmRegister |= (1U << static_cast<uint32_t>(alarmID));
    alarmLed = 1;
    displayOnScreen();
}

/*
 * display alarm register value
 */
void Alarm::display(CommandVector&  /*cv*/) const
{
    std::cout << "Alarms = 0x" << static_cast<unsigned int>(alarmRegister) << std::endl;
}

/*
 * clear alarms
 */
void Alarm::clear(CommandVector&  /*cv*/)
{
    alarmRegister = 0;
    alarmLed = 0;
    LOG_INFO("Alarms cleared");
}

/*
 * clear alarms - to be called from display menu
 */
void Alarm::clearFromMenu()
{
    CommandVector cv{};
    clear(cv);
    constexpr uint16_t Timeout = 5U;
    Menu::getInstance().displayMessage("alarms cleared", Timeout);
}

/*
* display alarms on screen
*/
void Alarm::displayOnScreen() const
{
    if(!Menu::getInstance().isDisplayEnabled())
    {   
        // displaying not allowed
        return;
    }
    const std::vector<const std::string> texts =
    {
        "W",    // I2C write
        "Wr",   // I2C write before read
        "wR",   // read after write
        "I"     // no gyroscope interrupt
    };
    constexpr uint8_t LimitX = 127U;
    Display::getInstance().setFont(static_cast<const uint8_t*>(FontTahoma11), false, LimitX);
    std::string text = "alarms:";
    if(alarmRegister != 0U)
    {
        constexpr uint32_t Bits32 = 32;
        for(uint8_t index=0; index < Bits32; index++)
        {
            if((alarmRegister & (1U << index)) != 0)
            {
                text += " ";
                text += texts[index];
            }
        }
    }
    else
    {
        text += " -";
    }
    constexpr uint8_t PosY = 13;
    Display::getInstance().print(0, PosY, text);
    Display::getInstance().update();
}