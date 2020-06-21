#ifndef YOKE_H_
#define YOKE_H_

#include "USBJoystick.h"
#include "I2CDevice.h"
#include "Console.h"
#include "Switch.h"
#include "Filter.h"
#include "HX711.h"
#include <mbed.h>

#define USB_VID     0x0483 //STElectronics
#define USB_PID     0x5711 //joystick in FS mode + 1
#define USB_VER     0x0001 //Nucleo Yoke IMU ver. 1

#define I2C2_SCL    PF_1
#define I2C2_SDA    PF_0

#define LSM6DS3_AG_ADD  0xD6
#define LSM6DS3_INT1    PD_0

template<typename T> struct Vector3D
{
    T X;
    T Y;
    T Z;
};

enum struct LSM6DS3reg : uint8_t
{
    INT1_CTRL = 0x0D,
    CTRL1_XL = 0x10,
    CTRL7_G = 0x16,
    OUT_X_L_G = 0x22,
};

enum struct HatSwitchMode
{
    HatMode,
    TrimMode
};

class Yoke
{
public:
    Yoke(events::EventQueue& eventQueue);
    void displayStatus(CommandVector cv);
private:
    void imuInterruptHandler(void) { eventQueue.call(callback(this, &Yoke::handler)); }
    void handler(void);
    void setJoystickButtons(void);
    void setJoystickHat(void);
    events::EventQueue& eventQueue;     // event queue of the main thread
    DigitalOut systemLed;               // yoke heartbeat LED
    uint32_t counter{0};                // counter of handler execution
    USBJoystick usbJoystick;            // USB HID joystick device
    InterruptIn imuInterruptSignal;     //IMU sensor interrupt signal
    I2C i2cBus;                         // I2C bus for IMU sensor
    I2CDevice sensorGA;                 // IMU gyroscope and accelerometer sensor
    Timeout imuIntTimeout;              // timeout of the IMU sensor interrupts
    Timer handlerTimer;                 // measures handler call period
    Vector3D<int16_t> gyroscopeData;    // raw data from gyroscope
    Vector3D<int16_t> accelerometerData;    // raw data from accelerometer
    Vector3D<float> angularRate;        // measured IMU sensor angular rate in rad/s
    Vector3D<float>  acceleration;      // measured IMU sensor acceleration in g
    const float AngularRateResolution = 500.0f * 3.14159265f / 180.0f / 32768.0f;   // 1-bit resolution of angular rate in rad/s
    const float AccelerationResolution = 2.0f / 32768.0f;   // 1-bit resolution of acceleration in g
    float sensorPitch{0.0f}, sensorRoll{0.0f}, sensorYaw{0.0f};             // orientation of the IMU sensor
    float sensorPitchVariability{0.0f}, sensorRollVariability{0.0f};
    float sensorPitchReference{0.0f}, sensorRollReference{0.0f};
    DigitalOut calibrationLed;
    JoystickData joystickData{0};
    DigitalIn flapsUpSwitch;
    DigitalIn flapsDownSwitch;
    DigitalIn gearUpSwitch;
    DigitalIn gearDownSwitch;
    DigitalIn redPushbutton;
    DigitalIn greenPushbutton;
    DigitalIn hatCenterSwitch;
    DigitalIn setSwitch;
    DigitalIn resetSwitch;
    DigitalIn leftToggle;
    DigitalIn rightToggle;
    AnalogIn throttlePotentiometer;
    AnalogIn propellerPotentiometer;
    AnalogIn mixturePotentiometer;
    AnalogIn joystickGainPotentiometer;
    AnalogIn tinyJoystickX;
    AnalogIn tinyJoystickY;
    Hat hatSwitch;
    HatSwitchMode hatMode{HatSwitchMode::TrimMode};
    FilterEMA joystickGainFilter;
    HX711 leftPedalTensometer;          // left pedal tensometer object
    HX711 rightPedalTensometer;         // right pedal tensometer object
};

#endif /* YOKE_H_ */