/*
 * Display.h
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <mbed.h>
#include "SH1106.h"

class Display
{
public:
    static Display& getInstance(void);
    Display(Display const&) = delete;   // copy constructor removed for singleton
    void operator=(Display const&) = delete;
    void init(void);
    void test(void);
    void update(void) { eventQueue.call(callback(&controller, &SH1106::update)); }
    void setFont(const uint8_t* newFont, bool newInvertion = false, uint8_t newXLimit = 0);
    void putChar(uint8_t X, uint8_t Y, uint8_t ch) { eventQueue.call(callback(&controller, &SH1106::putChar), X, Y, ch); } // displays character on the screen
    void print(uint8_t X, uint8_t Y, std::string text)  { eventQueue.call(callback(&controller, &SH1106::print), X, Y, text); } // displays string on the screen
private:
    Display(); // private constructor definition
    SH1106 controller;
    EventQueue eventQueue;
    Thread displayQueueDispatchThread;
};

#endif /* DISPLAY_H_ */