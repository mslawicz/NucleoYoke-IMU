/*
 * Console.cpp
 *
 *  Created on: 22.12.2019
 *      Author: Marcin
 */

#include "Console.h"
#include "mbed.h"
#include <iostream>
#include <type_traits>

Console& Console::getInstance()
{
    static Console instance;    // Guaranteed to be destroyed, instantiated on first use
    return instance;
}

/*
 * Console handler to be run in a separate thread
 */
void Console::handler()
{
    // start console execution with a delay
    constexpr Kernel::Clock::duration_u32 ConsoleDelay{500ms};
    ThisThread::sleep_for(ConsoleDelay);

    int ch{0};
    while(true)
    {
        std::string inputLine;
        std::cout << "\r>" << std::flush;

        do
        {
            ch = getchar();
            switch(ch)
            {
            case static_cast<int>(KeyCode::Backspace):
            case static_cast<int>(KeyCode::Delete):
                if(!inputLine.empty())
                {
                    putchar(static_cast<int>(KeyCode::Backspace));
                    putchar(' ');
                    putchar(static_cast<int>(KeyCode::Backspace));
                    fflush(stdout);
                    inputLine.pop_back();
                }
                break;
            default:
                if((ch >= static_cast<int>(KeyCode::Space)) && (ch < static_cast<int>(KeyCode::Delete)))
                {
                    inputLine.push_back(static_cast<char>(ch));
                    putchar(ch);
                    fflush(stdout);
                }
                break;
            }
        } while((ch != static_cast<int>(KeyCode::LF)) && (ch != static_cast<int>(KeyCode::CR)));

        commandElements.clear();
        size_t currentPosition{0};
        size_t nextSpacePosition{0};
        do
        {
            nextSpacePosition = inputLine.find(' ', currentPosition);
            auto word = inputLine.substr(currentPosition, nextSpacePosition - currentPosition);
            currentPosition = nextSpacePosition + 1;
            if(!word.empty())
            {
                commandElements.push_back(word);
            }
        } while(nextSpacePosition != std::string::npos);

        std::cout << std::endl;
        executeCommand();
    }
}

/*
 * register new command in the Console command map
 */
void Console::registerCommand(std::string command, const std::string& helpText,
        Callback<void(CommandVector&)> commandCallback)
{
    commands.emplace(command, CommandContainer{helpText, commandCallback});
}

/*
 * list all registered commands
 */
void Console::displayHelp(CommandVector&  /*cv*/)
{
    for(auto& command : commands)
    {
        std::cout << command.first << " - " << command.second.first << std::endl;
    }
}

/*
 * executes console command
 */
void Console::executeCommand()
{
    if((!commandElements.empty()) &&
       (!commandElements[0].empty()))
    {
        // command string is given
        auto commandIterator = commands.find(commandElements[0]);
        if(commandIterator != commands.end())
        {
            // this command has been registered - execute it
            commandIterator->second.second(commandElements);
        }
        else
        {
            // unknown command
            std::cout << "unknown command: " << commandElements[0] << std::endl;
        }
    }
}
