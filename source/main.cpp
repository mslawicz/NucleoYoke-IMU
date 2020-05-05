/* mbed Microcontroller Library
 * Copyright (c) 2006-2014 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Yoke.h"
#include "Console.h"
#include <mbed.h>


int main()
{
    printf("Nucleo Yoke IMU v1\r\n");

    // create and start console
    Console console;
    Thread consoleThread(osPriority_t::osPriorityLow4, OS_STACK_SIZE, nullptr, "console");
    // register console commands
    console.registerCommand("h", "help (display command list)", callback(&console, &Console::displayHelp));
    //console.registerCommand("lt", "list threads", callback(listThreads));
    //console.registerCommand("da", "display alarms", callback(&alarm, &Alarm::display));
    //console.registerCommand("ca", "clear alarms", callback(&alarm, &Alarm::clear));
    // start Console thread
    consoleThread.start(callback(&console, &Console::handler));

    // main event queue
    events::EventQueue eventQueue;

    // create Yoke object
    Yoke yoke(eventQueue);

    // process the event queue
    eventQueue.dispatch_forever();

    return 0;
}

