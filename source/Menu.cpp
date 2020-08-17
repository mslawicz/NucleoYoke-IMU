#include "Menu.h"

Menu::Menu() :
    menuQueueDispatchThread(osPriority_t::osPriorityLow4, OS_STACK_SIZE, nullptr, "menu"),
    execPushbutton(SwitchType::Pushbutton, PF_3, eventQueue),
    menuSelector(SwitchType::RotaryEncoder, PE_0, eventQueue, 0.01f, PF_11)
{
    // Start the display queue's dispatch thread
    menuQueueDispatchThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));

    execPushbutton.setCallback(callback(this, &Menu::execute));
    menuSelector.setCallback(callback(this, &Menu::changeItem));
}

Menu& Menu::getInstance(void)
{
    static Menu instance;    // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

/*
execute menu action on user button press
*/
void Menu::execute(uint8_t argument)
{
    printf("executing menu command\n");
}

/*
change active menu item
direction 0 (left) or 1 (right)
*/
void Menu::changeItem(uint8_t direction)
{
    printf("item change %d\n", direction);
}
