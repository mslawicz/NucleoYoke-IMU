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

struct JoystickData     //NOLINT(altera-struct-pack-align)
{
    int16_t X;
    int16_t Y;
    int16_t Z;
    int16_t Rx;
    int16_t Ry;
    int16_t Rz;
    int16_t slider;
    int16_t dial;
    uint8_t hat;
    uint32_t buttons;
};

class USBJoystick : public USBHID
{
public:
    USBJoystick(uint16_t vendorId, uint16_t productId, uint16_t productRelease, bool blocking = false);
    USBJoystick(USBJoystick const&) = delete;       // do not allow copy constructor of a singleton
    void operator=(USBJoystick const&) = delete;
    USBJoystick(USBJoystick&&) = delete;
    void operator=(USBJoystick&&) = delete;
     ~USBJoystick() override;
    const uint8_t* report_desc() override; // returns pointer to the report descriptor; Warning: this method must store the length of the report descriptor in reportLength
    bool sendReport(JoystickData& joystickData);
protected:
    const uint8_t* configuration_desc(uint8_t index) override;   // Get configuration descriptor; returns pointer to the configuration descriptor
    const uint8_t* string_iproduct_desc() override;      // Get string product descriptor
private:
    static constexpr size_t ConfigurationDescriptorSize = 41U;
    uint8_t configurationDescriptor[ConfigurationDescriptorSize]{0};        //NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

#endif /* USBJOYSTICK_H_ */