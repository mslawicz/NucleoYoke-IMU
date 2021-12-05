/*
 * Display.h
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "SH1106.h"
#include <mbed.h>
#include <utility>


class Display
{
public:
    static Display& getInstance();
    Display(Display const&) = delete;   // copy constructor removed for singleton
    void operator=(Display const&) = delete;
    Display(Display&&) = delete;
    void operator=(Display&&) = delete;
    void init();
    void test();
    void update() { eventQueue.call(callback(&controller, &SH1106::update)); }
    void setFont(const uint8_t* newFont, bool newInvertion = false, uint8_t newXLimit = 0);
    void putChar(uint8_t X, uint8_t Y, uint8_t ch) { eventQueue.call(callback(&controller, &SH1106::putChar), X, Y, ch); } // displays character on the screen
    void print(uint8_t X, uint8_t Y, std::string text)  { eventQueue.call(callback(&controller, &SH1106::print), X, Y, std::move(text)); } // displays string on the screen
    void setPoint(uint8_t X, uint8_t Y, bool clear = false);
    void drawRectangle(uint8_t X, uint8_t Y, uint8_t sizeX, uint8_t sizeY, bool clear = false);
    void drawLine(uint8_t fromX, uint8_t fromY, uint8_t toX, uint8_t toY, bool clear = false);
    void clear();
private:
    Display(); // private constructor definition
    ~Display() = default;
    SH1106 controller;
    EventQueue eventQueue;
    Thread displayQueueDispatchThread;
};

#endif /* DISPLAY_H_ */