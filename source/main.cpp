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
#include "Switch.h" //XXX test
#include <mbed.h>

void testCb(uint8_t direction)
{
    printf("encoder rotation=%u\r\n", direction);
}

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

    // main event queue
    events::EventQueue eventQueue;

    // create Yoke object
    Yoke yoke(eventQueue);

    //XXX test of encoder callback
    Switch testEncoder(SwitchType::BinaryEncoder, PG_3, eventQueue, 0.01f, PG_2);
    testEncoder.setCallback(testCb);

    // process the event queue
    eventQueue.dispatch_forever();

    return 0;
}

