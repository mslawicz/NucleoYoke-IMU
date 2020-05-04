/*
 * USBJoystick.h
 *
 *  Created on: 05.02.2020
 *      Author: Marcin
 */

#ifndef USBJOYSTICK_H_
#define USBJOYSTICK_H_

#include "USBHID.h"
#include <mbed.h>

struct JoystickData
{
    int16_t X;
    int16_t Y;
    int16_t Z;
    int16_t Rx;
    int16_t Ry;
    int16_t Rz;
    uint8_t hat;
    uint16_t buttons;
};

class USBJoystick : public USBHID
{
public:
    USBJoystick(uint16_t vendorId, uint16_t productId, uint16_t productRelease, bool blocking = false);
    virtual ~USBJoystick();
    const uint8_t* report_desc() override; // returns pointer to the report descriptor; Warning: this method must store the length of the report descriptor in reportLength
    bool sendReport(JoystickData& joystickData);
protected:
    const uint8_t* configuration_desc(uint8_t index) override;   // Get configuration descriptor; returns pointer to the configuration descriptor
    const uint8_t* string_iproduct_desc() override;      // Get string product descriptor
private:
    uint8_t configurationDescriptor[41];
};

#endif /* USBJOYSTICK_H_ */