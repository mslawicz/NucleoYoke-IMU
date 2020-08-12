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
#include "Alarm.h"
#include "Statistics.h"
#include "Display.h"
#include <mbed.h>
#include "Storage.h" //XXX test


int main()
{
    printf("Nucleo Yoke IMU v1\r\n");

    // create and start console thread
    Thread consoleThread(osPriority_t::osPriorityLow4, OS_STACK_SIZE, nullptr, "console");
    consoleThread.start(callback(&Console::getInstance(), &Console::handler));

    // register some console commands
    Console::getInstance().registerCommand("h", "help (display command list)", callback(&Console::getInstance(), &Console::displayHelp));
    Console::getInstance().registerCommand("lt", "list threads", callback(listThreads));

    // init display
    Display::getInstance().init();
    Display::getInstance().setFont(FontTahoma16b);
    Display::getInstance().print(2, 0, "Nucleo Yoke");
    Display::getInstance().update();

    Alarm::getInstance().displayOnScreen();


    //XXX test
    float x = 0.1f;
    printf("x=%f\n", x);
    std::string x_name = "/kv/variable_x";
    x = KvStore::getInstance().restore<float>(x_name, 3.14f);
    printf("x=%f\n", x);

    // main event queue
    events::EventQueue eventQueue;

    // create Yoke object
    Yoke yoke(eventQueue);

    // process the event queue
    eventQueue.dispatch_forever();

    return 0;
}

