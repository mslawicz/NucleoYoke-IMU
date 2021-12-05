#ifndef MENU_H_
#define MENU_H_

#include "Switch.h"
#include "Display.h"
#include <mbed.h>
#include <string>
#include <vector>

using MenuItem = std::pair<std::string, Callback<void(void)>>;

class Menu
{
public:
    static Menu& getInstance();
    Menu(Menu const&) = delete;   // copy constructor removed for singleton
    void operator=(Menu const&) = delete;
    Menu(Menu&&) = delete;
    void operator=(Menu&&) = delete;    
    void addItem(std::string itemText, Callback<void(void)> itemFunction) { menuItems.emplace_back(itemText, itemFunction); }
    void displayItemText();
    void displayMessage(std::string message, uint16_t timeout = 0, bool inverted = true);
    void clearMessage();
    void enableMenuChange() { isChangeItemEnabled = true; }
    void disableMenuChange() { isChangeItemEnabled = false; }
    void enableDisplay() { displayEnabled = true; }
    void disableDisplay() { displayEnabled = false; }
    bool isDisplayEnabled() const { return displayEnabled; }
private:
    Menu(); // private constructor definition
    ~Menu() = default;
    void execute(uint8_t argument);
    void changeItem(uint8_t direction);
    EventQueue eventQueue;
    Thread menuQueueDispatchThread;
    Switch execPushbutton;
    Switch menuSelector;
    std::vector<MenuItem> menuItems;
    uint8_t currentItem{0};
    const uint8_t MessageLine = 35;
    Timeout messageClearTimeout;
    bool isChangeItemEnabled{true};
    bool displayEnabled{true};
    const uint8_t* MenuFont = static_cast<const uint8_t*>(FontTahoma14b);
    const uint8_t MaxX = 127;
};

#endif /* MENU_H_ */