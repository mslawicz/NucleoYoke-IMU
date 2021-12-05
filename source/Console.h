/*
 * Console.h
 *
 *  Created on: 22.12.2019
 *      Author: Marcin
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "mbed.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

enum class KeyCode : int
{
    Backspace = 8,
    LF = 10,
    CR = 13,
    Escape = 27,
    Space = 32,
    Tilde = 126,
    Delete = 127
};

using CommandVector = std::vector<std::string>;
using CommandContainer = std::pair<std::string, Callback<void(CommandVector&)>>;

class Console
{
public:
    static Console& getInstance();
    Console(Console const&) = delete;   // copy constructor removed for singleton
    void operator=(Console const&) = delete;
    Console(Console&&) = delete;
    void operator=(Console&&) = delete;
    void handler();
    void registerCommand(std::string command, const std::string& helpText, Callback<void(CommandVector&)> commandCallback);
    void displayHelp(CommandVector& cv);
private:
    Console() = default; // private constructor definition
    ~Console() = default;
    void executeCommand();
    CommandVector commandElements;
    std::map<std::string, CommandContainer> commands;
};

#endif /* CONSOLE_H_ */
