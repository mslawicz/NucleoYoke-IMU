/*
 * Alarm.cpp
 *
 *  Created on: 13.04.2020
 *      Author: marci
 */

#include "Alarm.h"
#include "Display.h"
#include "Menu.h"

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
    alarmRegister |= (1 << static_cast<int>(alarmID));
    alarmLed = 1;
    displayOnScreen();
}

/*
 * display alarm register value
 */
void Alarm::display(CommandVector&  /*cv*/)
{
    printf("Alarms = 0x%08X\r\n", static_cast<unsigned int>(alarmRegister));
}

/*
 * clear alarms
 */
void Alarm::clear(CommandVector&  /*cv*/)
{
    alarmRegister = 0;
    alarmLed = 0;
    printf("Alarms cleared\r\n");
}

/*
 * clear alarms - to be called from display menu
 */
void Alarm::clearFromMenu(void)
{
    CommandVector cv{};
    clear(cv);
    Menu::getInstance().displayMessage("alarms cleared", 5);
}

/*
* display alarms on screen
*/
void Alarm::displayOnScreen(void)
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
    Display::getInstance().setFont(FontTahoma11, false, 127);
    std::string text = "alarms:";
    if(alarmRegister)
    {
        for(uint8_t index=0; index<32; index++)
        {
            if(alarmRegister & (1 << index))
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
    Display::getInstance().print(0, 13, text);
    Display::getInstance().update();
}