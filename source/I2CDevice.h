/*
 * I2CDevice.h
 *
 *  Created on: 17.02.2020
 *      Author: Marcin
 */

#ifndef I2CDEVICE_H_
#define I2CDEVICE_H_

#include <mbed.h>
#include <vector>

class I2CDevice
{
public:
    I2CDevice(I2C& bus, uint8_t deviceAddress);
    void write(uint8_t registerAddress, std::vector<uint8_t> data);
    std::vector<uint8_t> read(uint8_t registerAddress, uint8_t length);
private:
    I2C& bus;
    uint8_t address;
};

#endif /* I2CDEVICE_H_ */