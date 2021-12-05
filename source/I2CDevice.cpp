/*
 * I2CDevice.cpp
 *
 *  Created on: 17.02.2020
 *      Author: Marcin
 */

#include "I2CDevice.h"
#include "Alarm.h"

//extern Alarm alarm;

I2CDevice::I2CDevice(I2C& bus, uint8_t deviceAddress) :
    bus(bus),
    address(deviceAddress)
{
}

/*
 * write to I2C device
 */
void I2CDevice::write(uint8_t registerAddress, std::vector<uint8_t> data)
{
    // insert register address byte at the front
    data.insert(data.begin(), registerAddress);
    //if(bus.write(static_cast<int>(address), static_cast<const unsigned char*>(data.data()), data.size()) != 0)
    if(bus.write(static_cast<int>(address), reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size())) != 0)
    {
        Alarm::getInstance().set(AlarmID::I2CWrite);
    }
}

/*
 * read from I2C device
 */
std::vector<uint8_t> I2CDevice::read(uint8_t registerAddress, uint8_t length)
{
    std::vector<uint8_t> data(length);
    if(bus.write(static_cast<int>(address), reinterpret_cast<const char*>(&registerAddress), 1, true) != 0)
    {
        Alarm::getInstance().set(AlarmID::I2CWriteBeforeRead);
    }
    if(bus.read(static_cast<int>(address), reinterpret_cast<char*>(data.data()), length) != 0)
    {
        Alarm::getInstance().set(AlarmID::I2CReadAfterWrite);
    }
    return data;
}