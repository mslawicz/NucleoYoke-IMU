/*
 * Display.cpp
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#include "Display.h"

Display::Display() :
    controller(PE_14, PE_13, PE_12, PE_15, PF_13, PF_12),
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

/*
 * clear entire display
 */
void Display::clear(void)
{
    eventQueue.call(callback(&controller, &SH1106::clear));
}

/*
 * set point on display
 */
void Display::setPoint(uint8_t X, uint8_t Y, bool clear)
{
    eventQueue.call(callback(&controller, &SH1106::setPoint), X, Y, clear);
}

/*
 * draw rectangle
 */
void Display::drawRectangle(uint8_t X, uint8_t Y, uint8_t sizeX, uint8_t sizeY, bool clear)
{
    eventQueue.call(callback(&controller, &SH1106::drawRectangle), X, Y, sizeX, sizeY, clear);
}

/*
 * draw line between points
 */
void Display::drawLine(uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY, bool clear)
{
    eventQueue.call(callback(&controller, &SH1106::drawLine), fromX, toX, fromY, toY, clear);
}