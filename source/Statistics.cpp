/*
 * Statistics.cpp
 *
 *  Created on: 22.12.2019
 *      Author: Marcin
 */

#include "mbed_stats.h"
#include "Statistics.h"
#include <iostream>
#include <mbed.h>


#if !defined(MBED_THREAD_STATS_ENABLED)
#error "Stats not enabled"
#endif

void listThreads(CommandVector&  /*cv*/)
{
    auto* stats = new mbed_stats_thread_t[MAX_THREAD_STATS];     //NOLINT(cppcoreguidelines-owning-memory)
    size_t numberOfThreads = mbed_stats_thread_get_each(stats, MAX_THREAD_STATS);

    std::cout << "ID, Name, State, Priority, Stack size, Stack space" << std::endl;
    for(size_t i = 0; i < numberOfThreads; i++)
    {
        std::cout << "0x" << std::hex << stats[i].id << ", ";   //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::cout << stats[i].name << ", ";                     //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::cout << stats[i].state << ", ";                    //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::cout << stats[i].priority << ", ";                 //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::cout << stats[i].stack_size << ", ";               //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::cout << stats[i].stack_space << std::endl;         //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}