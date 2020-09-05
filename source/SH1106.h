/*
 * SH1106.h
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#ifndef SH1106_H_
#define SH1106_H_

#include "fonts.h"
#include <mbed.h>
#include <vector>
#include <string>

class SH1106
{
public:
    SH1106(PinName writeDataPin, PinName readDataPin, PinName clkPin, PinName resetPin, PinName cdPin, PinName csPin);
    void init(void);
    void update(void);
    void test(uint32_t argument);
    void setFont(const uint8_t* newFont, bool newInvertion = false, uint8_t newXLimit = 0);
    void putChar(uint8_t cX, uint8_t cY, uint8_t ch);
    void print(uint8_t sX, uint8_t sY, std::string text);
    void clear(void);
private:
    void write(uint8_t* data, int length, bool command = false);
    void write(std::vector<uint8_t>data, bool command = false) { write(&data[0], data.size(), command); }
    void setPoint(uint8_t X, uint8_t Y, bool clear = false);
    void putChar2CharSpace(void);
    SPI interface;
    DigitalOut resetSignal;
    DigitalOut cdSignal;
    DigitalOut csSignal;
    const std::vector<uint8_t> SH1106InitData
    {
        0x32,   //DC voltage output value
        0x40,   //display line for COM0 = 0
        0x81,   //contrast
        0x80,
        0xA1,   //segment left rotation
        0xA4,   //display in normal mode
        0xA6,   //display normal indication
        0xAD,   //DC pump on
        0x8B,
        0xC8    //scan from N-1 to 0
    };
    static const uint8_t sizeX = 128;
    static const uint8_t sizeY = 64;
    static const uint8_t noOfPages = 8;
    uint8_t dataBuffer[noOfPages][sizeX] = {0};
    uint8_t updateArray[noOfPages][2] = {{0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}, {0,sizeX-1}};
    const uint8_t* font{nullptr};      // pointer to font definition array
    bool inverted{false};   // display inverted characters
    uint8_t upToX{0};   // X limit of displayed pixels
    uint8_t X{0};       // current X coordinate
    uint8_t Y{0};       // current Y coordinate
};

#endif /* SH1106_H_ */
