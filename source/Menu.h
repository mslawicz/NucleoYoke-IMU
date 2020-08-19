#ifndef MENU_H_
#define MENU_H_

#include <mbed.h>
#include "Switch.h"
#include <string>
#include <vector>

using MenuItem = std::pair<std::string, Callback<void(void)>>;

class Menu
{
public:
    static Menu& getInstance(void);
    Menu(Menu const&) = delete;   // copy constructor removed for singleton
    void operator=(Menu const&) = delete;
    void addItem(std::string itemText, Callback<void(void)> itemFunction) { menuItems.emplace_back(itemText, itemFunction); }
    void displayItemText(void);
    void displayMessage(std::string message, uint16_t timeout = 0, bool inverted = true);
private:
    Menu(); // private constructor definition
    void execute(uint8_t argument);
    void changeItem(uint8_t direction);
    void clearMessage(void);
    EventQueue eventQueue;
    Thread menuQueueDispatchThread;
    Switch execPushbutton;
    Switch menuSelector;
    std::vector<MenuItem> menuItems;
    uint8_t currentItem{0};
    const uint8_t MessageLine = 35;
    Timeout messageClearTimeout;
};

#endif /* MENU_H_ */