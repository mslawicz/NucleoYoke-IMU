#ifndef YOKE_H_
#define YOKE_H_

#include "USBJoystick.h"
#include "I2CDevice.h"
#include "Console.h"
#include "Switch.h"
#include "Filter.h"
#include <mbed.h>

#define USB_VID     0x0483 //STElectronics
#define USB_PID     0x5711 //joystick in FS mode + 1
#define USB_VER     0x0001 //Nucleo Yoke IMU ver. 1

#define I2C2_SCL    PF_1
#define I2C2_SDA    PF_0

#define LSM6DS3_AG_ADD  0xD6
#define LSM6DS3_INT1    PD_1
#define LSM9DS1_AG_ADD  0xD6
#define LSM9DS1_M_ADD   0x3C
#define LSM9DS1_INT1    PD_1

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

enum struct LSM9DS1reg : uint8_t
{
    INT1_CTRL = 0x0C,
    CTRL_REG1_G = 0x10,
    OUT_X_L_G = 0x18,
    CTRL_REG6_XL = 0x20,
    CTRL_REG1_M = 0x20,
    OUT_X_L_M = 0x28
};

enum struct HatSwitchMode
{
    FreeViewMode,
    QuickViewMode,
    TrimMode
};

enum struct YokeMode
{
    FixedWing,
    Helicopter,
    Size
};

class Yoke
{
public:
    Yoke(events::EventQueue& eventQueue);
    void displayStatus(CommandVector& cv);
    void displayAll(void);
private:
    void imuInterruptHandler(void) { eventQueue.call(callback(this, &Yoke::handler)); }
    void handler(void);
    void setJoystickButtons(void);
    void axisCalibration(void);
    void toggleAxisCalibration(void);
    void toggleStopwatch(void);
    void displayMode(void);
    void displayStopwatch(void);
    events::EventQueue& eventQueue;     // event queue of the main thread
    DigitalOut systemLed;               // yoke heartbeat LED
    uint32_t counter{0};                // counter of handler execution
    USBJoystick usbJoystick;            // USB HID joystick device
    InterruptIn imuInterruptSignal;     //IMU sensor interrupt signal
    I2C i2cBus;                         // I2C bus for IMU sensor
    I2CDevice sensorGA;                 // IMU gyroscope and accelerometer sensor
    I2CDevice sensorM;                  // magnetometer sensor
    Timeout imuIntTimeout;              // timeout of the IMU sensor interrupts
    Timer handlerTimer;                 // measures handler call period
    Vector3D<int16_t> gyroscopeData;    // raw data from gyroscope
    Vector3D<int16_t> accelerometerData;    // raw data from accelerometer
    Vector3D<int16_t> magnetometerData; // raw data from magnetometer
    Vector3D<float> angularRate;        // measured IMU sensor angular rate in rad/s
    Vector3D<float>  acceleration;      // measured IMU sensor acceleration in g
    Vector3D<float>  magneticField;     // measured magnetometer sensor magnetic field in gauss
    const float AngularRateResolution = 500.0f * 3.14159265f / 180.0f / 32768.0f;   // 1-bit resolution of angular rate in rad/s
    const float AccelerationResolution = 2.0f / 32768.0f;   // 1-bit resolution of acceleration in g
    const float MagneticFieldResolution = 16.0f / 32768.0f;   // 1-bit resolution of magnetic field in gauss
    float sensorPitch{0.0f}, sensorRoll{0.0f}, sensorYaw{0.0f};             // orientation of the IMU sensor
    float sensorPitchVariability{0.0f}, sensorRollVariability{0.0f}, sensorYawVariability{0.0f};
    float sensorPitchReference, sensorRollReference, sensorYawReference;
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
    DigitalIn reverserSwitch;
    DigitalIn speedBrakeSwitch;
    DigitalIn hatModeToggle;
    DigitalIn viewModeToggle;
    DigitalIn hatModeShift;
    AnalogIn throttlePotentiometer;
    AnalogIn propellerPotentiometer;
    AnalogIn mixturePotentiometer;
    AnalogIn orangePotentiometer;
    AnalogIn yellowPotentiometer;
    AnalogIn blueGrayPotentiometer;
    Hat hatSwitch;
    HatSwitchMode hatMode{HatSwitchMode::TrimMode};
    FilterEMA joystickGainFilter;
    YokeMode yokeMode;
    bool isCalibrationOn{false};
    float throttleInput;
    float throttleInputMin;
    float throttleInputMax;
    bool isStopwatchDisplayed{false};
    Timer stopwatch;
    Ticker stopwatchTicker;
    FilterAEMA throttleFilter;
    FilterAEMA mixtureFilter;
    FilterAEMA propellerFilter;
    const std::vector<const std::string> modeTexts =
    {
        "fixed-wing",
        "helicopter"
    };
};

#endif /* YOKE_H_ */