#ifndef MENU_H_
#define MENU_H_

#include <mbed.h>
#include "Switch.h"

class Menu
{
public:
    static Menu& getInstance(void);
    Menu(Menu const&) = delete;   // copy constructor removed for singleton
    void operator=(Menu const&) = delete;
    void addItem(void) {} //XXX to implement later
private:
    Menu(); // private constructor definition
    void execute(uint8_t argument);
    void changeItem(uint8_t direction);
    EventQueue eventQueue;
    Thread menuQueueDispatchThread;
    Switch execPushbutton;
    Switch menuSelector;
};

#endif /* MENU_H_ */