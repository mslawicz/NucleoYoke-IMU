#include "Menu.h"

#include <utility>

Menu::Menu() :
    menuQueueDispatchThread(osPriority_t::osPriorityLow4, OS_STACK_SIZE, nullptr, "menu"),
    execPushbutton(SwitchType::Pushbutton, PF_3, eventQueue),
    menuSelector(SwitchType::RotaryEncoder, PE_0, eventQueue, 0.01F, PF_11)     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
{
    // Start the display queue's dispatch thread
    menuQueueDispatchThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));

    execPushbutton.setCallback(callback(this, &Menu::execute));
    menuSelector.setCallback(callback(this, &Menu::changeItem));
}

Menu& Menu::getInstance()
{
    static Menu instance;    // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

/*
execute menu action on user button press
*/
void Menu::execute(uint8_t  /*argument*/)
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
    if(isChangeItemEnabled && !menuItems.empty())
    {
        uint8_t noOfItems = menuItems.size();
        currentItem = (currentItem + direction * 2 - 1 + noOfItems) % noOfItems;
        displayItemText();
    }
}

/*
displays menu item text
*/
void Menu::displayItemText()
{
    if(!menuItems.empty() && (menuItems.size() > currentItem))
    {
        std::string text = ">" + menuItems[currentItem].first;
        Display::getInstance().setFont(MenuFont, false, MaxX);
        Display::getInstance().print(0, 49, text);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        Display::getInstance().update();
    }
}

/*
displays the message in a separate line
use timeout in [s] to clear automatically clear the message
*/
void Menu::displayMessage(std::string message, uint16_t timeout, bool inverted)
{
    Display::getInstance().setFont(MenuFont, inverted, MaxX);
    Display::getInstance().print(0, MessageLine, std::move(message));
    Display::getInstance().update();
    if(0 != timeout)
    {
        constexpr uint32_t UsInSec = 1000000;
        messageClearTimeout.attach(callback(this, &Menu::clearMessage), std::chrono::microseconds(timeout * UsInSec));
    }
}

/*
displays the message in a separate line
use timeout in [s] to clear automatically clear the message
*/
void Menu::clearMessage()
{
    Display::getInstance().setFont(MenuFont, false, MaxX);
    Display::getInstance().print(0, MessageLine, " ");
    Display::getInstance().update();
}