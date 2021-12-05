/*
 * USBJoystick.cpp
 *
 *  Created on: 05.02.2020
 *      Author: Marcin
 */

#include "USBJoystick.h"
#include "Convert.h"
#include "Logger.h"
#include "usb_phy_api.h"
#include <iomanip>

USBJoystick::USBJoystick(uint16_t vendorId, uint16_t productId, uint16_t productRelease, bool blocking) :
    USBHID(get_usb_phy(), 0, 0, vendorId, productId, productRelease)
{
    if (blocking)
    {
        LOG_INFO(std::hex << setfill('0') << setw(4) << "Connecting USB HID joystick device (VID=0x" << vendorId << ", PID=0x" << productId
                << std::dec << ", VER=" << productRelease << ") in blocking mode");
        USBDevice::connect();
        wait_ready();
    }
    else
    {
        LOG_INFO(std::hex << setfill('0') << setw(4) << "Initializing USB HID joystick device (VID=0x" << vendorId << ", PID=0x" << productId
        << std::dec << ", VER=" << productRelease << ") in non-blocking mode");
        init();
    }    
}

USBJoystick::~USBJoystick()
{
    deinit();
}

const uint8_t* USBJoystick::report_desc()
{
    static const uint8_t report_descriptor[] =      //NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
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
    return static_cast<const uint8_t*>(report_descriptor);
}

const uint8_t* USBJoystick::configuration_desc(uint8_t index)
{
    if (index != 0)
    {
        return nullptr;
    }

    const uint8_t DefaultConfiguration = 1;
    const uint16_t TotalDescriptorLength = CONFIGURATION_DESCRIPTOR_LENGTH + INTERFACE_DESCRIPTOR_LENGTH
                                         + HID_DESCRIPTOR_LENGTH + 2 * ENDPOINT_DESCRIPTOR_LENGTH; 
    constexpr uint8_t BmAttributes = (1U << 6U) | (1U << 7U);   
    const uint16_t HidVersion = HID_VERSION_1_11;
    uint16_t reportDescriptorLength = report_desc_length();
    const uint16_t MaxHIDReportSize = MAX_HID_REPORT_SIZE;
    const uint8_t PollInterval = 1;     // host polling interval [ms]

    uint8_t configurationDescriptorTemp[] =     //NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    {
        CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
        CONFIGURATION_DESCRIPTOR,           // bDescriptorType
        LO8(TotalDescriptorLength),         // wTotalLength (LSB)
        HI8(TotalDescriptorLength),         // wTotalLength (MSB)
        0x01,                               // bNumInterfaces
        DefaultConfiguration,               // bConfigurationValue
        0x00,                               // iConfiguration
        BmAttributes,                       // bmAttributes
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
        LO8(HidVersion),                    // bcdHID (LO8)
        HI8(HidVersion),                    // bcdHID (HI8)
        0x00,                               // bCountryCode
        0x01,                               // bNumDescriptors
        REPORT_DESCRIPTOR,                  // bDescriptorType
        LO8(reportDescriptorLength),        // wDescriptorLength (LO8)
        HI8(reportDescriptorLength),        // wDescriptorLength (HI8)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_in,                            // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LO8(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        HI8(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1,                                  // bInterval (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_out,                           // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LO8(MaxHIDReportSize),              // wMaxPacketSize (LSB)
        HI8(MaxHIDReportSize),              // wMaxPacketSize (MSB)
        PollInterval                        // bInterval (milliseconds)
    };

    MBED_ASSERT(sizeof(configurationDescriptorTemp) == sizeof(configurationDescriptor));
    memcpy(static_cast<void*>(configurationDescriptor), static_cast<void*>(configurationDescriptorTemp), sizeof(configurationDescriptor));
    return static_cast<const uint8_t*>(configurationDescriptor);
}

/*
* Get string product descriptor
*
* @returns pointer to the string product descriptor
*/
const uint8_t* USBJoystick::string_iproduct_desc()
{
    static const uint8_t OverriddenStringIproductDescriptor[] = {   //NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
        0x18,                                                       //bLength
        STRING_DESCRIPTOR,                                          //bDescriptorType 0x03
        'N', 0, 'u', 0, 'c', 0, 'l', 0, 'e', 0, 'o', 0, ' ', 0, 'Y', 0, 'o', 0, 'k', 0, 'e', 0 //bString iProduct - HID device
    };
    return static_cast<const uint8_t*>(OverriddenStringIproductDescriptor);
}

/*
 * sends HID joystick report to PC
 */
bool USBJoystick::sendReport(JoystickData& joystickData)
{
    HID_REPORT report;
    uint8_t index = 0;
    report.data[index++] = LO8(joystickData.X);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.X);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.Y);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.Y);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.Z);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.Z);         //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.Rz);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.Rz);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.Rx);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.Rx);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.Ry);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.Ry);        //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.slider);    //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.slider);    //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = LO8(joystickData.dial);      //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = HI8(joystickData.dial);      //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = joystickData.hat;            //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    report.data[index++] = joystickData.buttons & 0xFF;                     //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index,hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
    report.data[index++] = (joystickData.buttons >> 8) & 0xFF;              //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index,hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
    report.data[index++] = (joystickData.buttons >> 16) & 0xFF;             //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index,hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)
    report.data[index++] = (joystickData.buttons >> 24) & 0xFF;             //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index,hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,readability-magic-numbers)

    report.length = index;
    return send(&report);
}
