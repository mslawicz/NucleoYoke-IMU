#ifndef YOKE_H_
#define YOKE_H_

#include "USBJoystick.h"
#include "I2CDevice.h"
#include <mbed.h>

#define USB_VID     0x0483 //STElectronics
#define USB_PID     0x5711 //joystick in FS mode + 1
#define USB_VER     0x0001 //Nucleo Yoke IMU ver. 1

#define I2C1_SCL    PB_8
#define I2C1_SDA    PB_9

#define LSM6DS3_AG_ADD  0xD6
#define LSM6DS3_INT1    PC_8

enum struct LSM6DS3reg : uint8_t
{
    INT1_CTRL = 0x0D,
    CTRL1_XL = 0x10,
    CTRL7_G = 0x16,
    OUT_X_L_G = 0x22,
};

class Yoke
{
public:
    Yoke(events::EventQueue& eventQueue);
private:
    void imuInterruptHandler(void) { eventQueue.call(callback(this, &Yoke::handler)); }
    void handler(void);
    events::EventQueue& eventQueue;     // event queue of the main thread
    DigitalOut systemLed;               // yoke heartbeat LED
    uint32_t counter{0};                // counter of handler execution
    USBJoystick usbJoystick;            // USB HID joystick device
    InterruptIn imuInterruptSignal;     //IMU sensor interrupt signal
    I2C i2cBus;                         // I2C bus for IMU sensor
    I2CDevice sensorGA;                 // IMU gyroscope and accelerometer sensor
    Timeout imuIntTimeout;              // timeout of the IMU sensor interrupts
    Timer handlerTimer;                 // measures handler call period
};

#endif /* YOKE_H_ */