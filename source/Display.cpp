/*
 * Display.cpp
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#include "Display.h"

Display::Display() :
    controller(PE_14, PE_12, PE_15, PE_13, PF_13),
    displayQueueDispatchThread(osPriority_t::osPriorityBelowNormal, OS_STACK_SIZE, nullptr, "display")
{
    // Start the display queue's dispatch thread
    displayQueueDispatchThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));
}

Display& Display::getInstance(void)
{
    static Display instance;    // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

/*
 * initialize display
 */
void Display::init(void)
{
    eventQueue.call(callback(&controller, &SH1106::init));
}

/*
 * call display test function
 */
void Display::test(void)
{
    uint8_t argument = 30;
    eventQueue.call(callback(&controller, &SH1106::test), argument);
}

/*
 * set new font parameters
 * font - font array from fonts.h
 * inverted - clears pixels if true
 * upToX - if >0, stops at X==upToX
 */
void Display::setFont(const uint8_t* newFont, bool newInvertion, uint8_t newXLimit)
{
    eventQueue.call(callback(&controller, &SH1106::setFont), newFont, newInvertion, newXLimit);
}
