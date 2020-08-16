#ifndef MENU_H_
#define MENU_H_

#include <mbed.h>

class Menu
{
public:
    static Menu& getInstance(void);
    Menu(Menu const&) = delete;   // copy constructor removed for singleton
    void operator=(Menu const&) = delete;
private:
    Menu(); // private constructor definition
    EventQueue eventQueue;
    Thread menuQueueDispatchThread;
};

#endif /* MENU_H_ */