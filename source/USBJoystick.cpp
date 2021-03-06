/*
 * USBJoystick.cpp
 *
 *  Created on: 05.02.2020
 *      Author: Marcin
 */

#include "USBJoystick.h"
#include "usb_phy_api.h"

USBJoystick::USBJoystick(uint16_t vendorId, uint16_t productId, uint16_t productRelease, bool blocking) :
    USBHID(get_usb_phy(), 0, 0, vendorId, productId, productRelease)
{
    if (blocking)
    {
        printf("Connecting USB HID joystick device (VID=0x%04X, PID=0x%04X, VER=%u) in blocking mode\r\n", vendorId, productId, productRelease);
        USBDevice::connect();
        wait_ready();
    }
    else
    {
        printf("Initializing USB HID joystick device (VID=0x%04X, PID=0x%04X, VER=%u) in non-blocking mode\r\n", vendorId, productId, productRelease);
        init();
    }
}

USBJoystick::~USBJoystick()
{
    deinit();
}

const uint8_t* USBJoystick::report_desc()
{
    static const uint8_t report_descriptor[] =
    {
        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
        0x09, 0x04,                    // USAGE (Joystick)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
        0x09, 0x01,                    //   USAGE (Pointer)
        0xa1, 0x00,                    //   COLLECTION (Physical)
        0x75, 0x10,                    //     REPORT_SIZE (16)
        0x16, 0x01, 0x80,              //     LOGICAL_MINIMUM (-32767)
        0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
        0x09, 0x30,                    //     USAGE (X)
        0x09, 0x31,                    //     USAGE (Y)
        0x09, 0x32,                    //     USAGE (Z)
        0x09, 0x35,                    //     USAGE (Rz)
        0x95, 0x04,                    //     REPORT_COUNT (4)
        0x81, 0x02,                    //     INPUT (Data,Var,Abs)
        0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
        0x09, 0x33,                    //     USAGE (Rx)
        0x09, 0x34,                    //     USAGE (Ry)
        0x09, 0x36,                    //     USAGE (slider)
        0x09, 0x37,                    //     USAGE (dial)
        0x95, 0x04,                    //     REPORT_COUNT (4)
        0x81, 0x02,                    //     INPUT (Data,Var,Abs)
        0xc0,                          //   END_COLLECTION
        0x09, 0x39,                    //   USAGE (Hat switch)
        0x15, 0x01,                    //   LOGICAL_MINIMUM (1)
        0x25, 0x08,                    //   LOGICAL_MAXIMUM (8)
        0x35, 0x00,                    //   PHYSICAL_MINIMUM (0)
        0x46, 0x3b, 0x01,              //   PHYSICAL_MAXIMUM (315)
        0x65, 0x14,                    //   UNIT (Eng Rot:Angular Pos)
        0x75, 0x04,                    //   REPORT_SIZE (4)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x81, 0x42,                    //   INPUT (Data,Var,Abs, Null)
        0x75, 0x04,                    //   REPORT_SIZE (4)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x81, 0x41,                    //   INPUT (Cnst,Ary,Abs,Null)
        0x05, 0x09,                    //   USAGE_PAGE (Button)
        0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
        0x29, 0x20,                    //   USAGE_MAXIMUM (Button 32)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x95, 0x20,                    //   REPORT_COUNT (16)
        0x55, 0x00,                    //   UNIT_EXPONENT (0)
        0x65, 0x00,                    //   UNIT (None)
        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
        0xc0                           // END_COLLECTION
    };

    reportLength = sizeof(report_descriptor);
    return report_descriptor;
}

#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

const uint8_t* USBJoystick::configuration_desc(uint8_t index)
{
    if (index != 0)
    {
        return nullptr;
    }

    uint8_t configurationDescriptorTemp[] =
    {
        CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
        CONFIGURATION_DESCRIPTOR,           // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (MSB)
        0x01,                               // bNumInterfaces
        DEFAULT_CONFIGURATION,              // bConfigurationValue
        0x00,                               // iConfiguration
        C_RESERVED | C_SELF_POWERED,        // bmAttributes
        C_POWER(0),                         // bMaxPower
        /************** Descriptor of joystick interface ****************/
        INTERFACE_DESCRIPTOR_LENGTH,        // bLength
        INTERFACE_DESCRIPTOR,               // bDescriptorType
        0x00,                               // bInterfaceNumber
        0x00,                               // bAlternateSetting
        0x01,                               // bNumEndpoints
        HID_CLASS,                          // bInterfaceClass
        HID_SUBCLASS_NONE,                  // bInterfaceSubClass
        HID_PROTOCOL_NONE,                  // bInterfaceProtocol
        0x00,                               // iInterface

        HID_DESCRIPTOR_LENGTH,              // bLength
        HID_DESCRIPTOR,                     // bDescriptorType
        LSB(HID_VERSION_1_11),              // bcdHID (LSB)
        MSB(HID_VERSION_1_11),              // bcdHID (MSB)
        0x00,                               // bCountryCode
        0x01,                               // bNumDescriptors
        REPORT_DESCRIPTOR,                  // bDescriptorType
        (uint8_t) (LSB(report_desc_length())), // wDescriptorLength (LSB)
        (uint8_t) (MSB(report_desc_length())), // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_in,                            // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1,                                  // bInterval (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_out,                           // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1                                  // bInterval (milliseconds)
    };

    MBED_ASSERT(sizeof(configurationDescriptorTemp) == sizeof(configurationDescriptor));
    memcpy(configurationDescriptor, configurationDescriptorTemp, sizeof(configurationDescriptor));
    return configurationDescriptor;
}

/*
* Get string product descriptor
*
* @returns pointer to the string product descriptor
*/
const uint8_t* USBJoystick::string_iproduct_desc()
{
    static const uint8_t OverriddenStringIproductDescriptor[] = {
        0x18,                                                       //bLength
        STRING_DESCRIPTOR,                                          //bDescriptorType 0x03
        'N', 0, 'u', 0, 'c', 0, 'l', 0, 'e', 0, 'o', 0, ' ', 0, 'Y', 0, 'o', 0, 'k', 0, 'e', 0 //bString iProduct - HID device
    };
    return OverriddenStringIproductDescriptor;
}

/*
 * sends HID joystick report to PC
 */
bool USBJoystick::sendReport(JoystickData& joystickData)
{
    HID_REPORT report;
    uint8_t index = 0;
    report.data[index++] = LSB(joystickData.X);
    report.data[index++] = MSB(joystickData.X);
    report.data[index++] = LSB(joystickData.Y);
    report.data[index++] = MSB(joystickData.Y);
    report.data[index++] = LSB(joystickData.Z);
    report.data[index++] = MSB(joystickData.Z);
    report.data[index++] = LSB(joystickData.Rz);
    report.data[index++] = MSB(joystickData.Rz);
    report.data[index++] = LSB(joystickData.Rx);
    report.data[index++] = MSB(joystickData.Rx);
    report.data[index++] = LSB(joystickData.Ry);
    report.data[index++] = MSB(joystickData.Ry);
    report.data[index++] = LSB(joystickData.slider);
    report.data[index++] = MSB(joystickData.slider);
    report.data[index++] = LSB(joystickData.dial);
    report.data[index++] = MSB(joystickData.dial);
    report.data[index++] = joystickData.hat;
    report.data[index++] = joystickData.buttons & 0xFF;
    report.data[index++] = (joystickData.buttons >> 8) & 0xFF;
    report.data[index++] = (joystickData.buttons >> 16) & 0xFF;
    report.data[index++] = (joystickData.buttons >> 24) & 0xFF;

    report.length = index;
    return send(&report);
}
