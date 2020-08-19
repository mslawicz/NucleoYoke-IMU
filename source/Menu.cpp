#include "Menu.h"
#include "Display.h"

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
    if(!menuItems.empty() &&
      (menuItems.size() > currentItem) &&
      menuItems[currentItem].second)
      {
          menuItems[currentItem].second();
      }
}

/*
change active menu item
direction 0 (left) or 1 (right)
*/
void Menu::changeItem(uint8_t direction)
{
    if(!menuItems.empty())
    {
        uint8_t noOfItems = menuItems.size();
        currentItem = (currentItem + direction * 2 - 1 + noOfItems) % noOfItems;
        displayItemText();
    }
}

/*
displays menu item text
*/
void Menu::displayItemText(void)
{
    if(!menuItems.empty() && (menuItems.size() > currentItem))
    {
        std::string text = ">" + menuItems[currentItem].first;
        Display::getInstance().setFont(FontTahoma14b, false, 127);
        Display::getInstance().print(0, 49, text);
        Display::getInstance().update();
    }
}

/*
displays the message in a separate line
use timeout in [s] to clear automatically clear the message
*/
void Menu::displayMessage(std::string message, uint16_t timeout)
{
    Display::getInstance().setFont(FontTahoma14b, true, 127);
    Display::getInstance().print(0, MessageLine, message);
    Display::getInstance().update();
    if(timeout)
    {
        messageClearTimeout.attach(callback(this, &Menu::clearMessage), std::chrono::microseconds(timeout * 1000000));
    }
}

/*
displays the message in a separate line
use timeout in [s] to clear automatically clear the message
*/
void Menu::clearMessage(void)
{
    Display::getInstance().setFont(FontTahoma14b, false, 127);
    Display::getInstance().print(0, MessageLine, " ");
    Display::getInstance().update();
}