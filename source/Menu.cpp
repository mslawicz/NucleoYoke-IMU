#include "Menu.h"

Menu::Menu() :
    menuQueueDispatchThread(osPriority_t::osPriorityLow4, OS_STACK_SIZE, nullptr, "menu")
{
    // Start the display queue's dispatch thread
    menuQueueDispatchThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));
}

Menu& Menu::getInstance(void)
{
    static Menu instance;    // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

