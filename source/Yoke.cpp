#include "Yoke.h"
#include "Scale.h"
#include "Alarm.h"
#include "Storage.h"
#include "Display.h"

//XXX global variables for test
float g_gyroX, g_gyroY, g_gyroZ;
float g_accX, g_accY, g_accZ;
float g_magX, g_magY, g_magZ;
float g_sensorPitch, g_sensorRoll, g_sensorYaw;

static const float PI = 3.14159265359f;

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED2),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    imuInterruptSignal(LSM9DS1_INT1, PullDown),
    i2cBus(I2C2_SDA, I2C2_SCL),
    sensorGA(i2cBus, LSM9DS1_AG_ADD),
    sensorM(i2cBus, LSM9DS1_M_ADD),
    calibrationLed(LED1, 0),
    flapsUpSwitch(PB_15, PullUp),
    flapsDownSwitch(PB_13, PullUp),
    gearUpSwitch(PF_4, PullUp),
    gearDownSwitch(PF_5, PullUp),
    redPushbutton(PG_0, PullUp),
    greenPushbutton(PB_2, PullUp),
    hatCenterSwitch(PG_15, PullUp),
    setSwitch(PE_1, PullUp),
    resetSwitch(PE_6, PullUp),
    leftToggle(PF_9, PullUp),
    rightToggle(PF_8, PullUp),
    reverserSwitch(PG_3, PullUp),
    throttlePotentiometer(PA_0),
    propellerPotentiometer(PA_4),
    mixturePotentiometer(PA_1),
    orangePotentiometer(PC_0),
    yellowPotentiometer(PC_1),
    hatSwitch(PG_13, PG_9, PG_12, PG_10),
    joystickGainFilter(0.01f),
    menuSelector(SwitchType::RotaryEncoder, PB_10/*, eventQueue*/, 0.01f, PC_12)
{
    printf("Yoke object created\r\n");

    i2cBus.frequency(400000);

    // connect USB joystick
    usbJoystick.connect();

    // configure IMU sensor
    // Gyroscope ODR=119 Hz, full scale 500 dps
    // int/out selection default
    // low power disable, HPF enable, HPF=0.05 Hz
    sensorGA.write((uint8_t)LSM9DS1reg::CTRL_REG1_G, std::vector<uint8_t>{0x68, 0x00, 0x47});
    // Accelerometer ODR=119 Hz, full scale +=2g
    sensorGA.write((uint8_t)LSM9DS1reg::CTRL_REG6_XL, std::vector<uint8_t>{0x60});
    // INT1<-DRDY_G
    sensorGA.write((uint8_t)LSM9DS1reg::INT1_CTRL, std::vector<uint8_t>{0x02});

    // configure magnetometer sensor
    // Magnetometer X&Y high-performance mode, ODR=80 Hz
    // full scale +-16 gauss
    // continues conversion mode
    // Z-axis high-performance mode
    sensorM.write((uint8_t)LSM9DS1reg::CTRL_REG1_M, std::vector<uint8_t>{0x5C, 0x60, 0x00, 0x80});

    // restore yoke parameters
    yokeMode = KvStore::getInstance().restore<YokeMode>("/kv/yokeMode", YokeMode::FixedWing,
        static_cast<YokeMode>(0),
        static_cast<YokeMode>(static_cast<int>(YokeMode::Size) - 1));

    // call handler on IMU interrupt rise signal
    imuInterruptSignal.rise(callback(this, &Yoke::imuInterruptHandler));
    // this timeout calls handler for the first time
    // next calls will be executed upon IMU INT1 interrupt signal
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 100ms);

    // start handler timer
    handlerTimer.start();

    Console::getInstance().registerCommand("ys", "display yoke status", callback(this, &Yoke::displayStatus));
}



/*
* yoke handler is called on IMU interrupt signal
*/
void Yoke::handler(void)
{
    // this timeout is set only for the case of lost IMU interrupt signal
    // the timeout should never happen, as the next interrupt should be called earlier
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 20ms);

    // measure time elapsed since the previous call
    float deltaT = std::chrono::duration<float>(handlerTimer.elapsed_time()).count();
    handlerTimer.reset();
    counter++;

    // set HAT switch mode
    hatMode = leftToggle.read() ? HatSwitchMode::TrimMode : HatSwitchMode::HatMode;

    // set brake mode from SET pushbutton
    bool brakeActive = !setSwitch.read();

    std::vector<uint8_t> sensorData;
    if(imuInterruptSignal.read() == 1)
    {
        // interrupt signal is active
        // read IMU sensor data
        sensorData = sensorGA.read((uint8_t)LSM9DS1reg::OUT_X_L_G, 12);
        gyroscopeData.X = *reinterpret_cast<int16_t*>(&sensorData[4]);
        gyroscopeData.Y = *reinterpret_cast<int16_t*>(&sensorData[2]);
        gyroscopeData.Z = *reinterpret_cast<int16_t*>(&sensorData[0]);
        accelerometerData.X = *reinterpret_cast<int16_t*>(&sensorData[10]);
        accelerometerData.Y = *reinterpret_cast<int16_t*>(&sensorData[8]);
        accelerometerData.Z = *reinterpret_cast<int16_t*>(&sensorData[6]);

        // read magnetometer data
        sensorData = sensorM.read((uint8_t)LSM9DS1reg::OUT_X_L_M, 6);
        magnetometerData.X = *reinterpret_cast<int16_t*>(&sensorData[0]);
        magnetometerData.Y = *reinterpret_cast<int16_t*>(&sensorData[2]);
        magnetometerData.Z = *reinterpret_cast<int16_t*>(&sensorData[4]);
    }
    else
    {
        // no interrupt signal
        Alarm::getInstance().set(AlarmID::NoImuInterrupt);
    }

    // calculate IMU sensor physical values; using right hand rule
    // X = roll axis = pointing North
    // Y = pitch axis = pointing East
    // Z = yaw axis = pointing down
    // angular rate in rad/s
    angularRate.X = -AngularRateResolution * gyroscopeData.Y;
    angularRate.Y = -AngularRateResolution * gyroscopeData.Z;
    angularRate.Z = AngularRateResolution * gyroscopeData.X;
    // acceleration in g
    acceleration.X = AccelerationResolution * accelerometerData.Z;
    acceleration.Y = -AccelerationResolution * accelerometerData.Y;
    acceleration.Z = -AccelerationResolution * accelerometerData.X;
    // magnetic field in gauss
    magneticField.X = MagneticFieldResolution * magnetometerData.X;
    magneticField.Y = MagneticFieldResolution * magnetometerData.Y;
    magneticField.Z = MagneticFieldResolution * magnetometerData.Z;

    float accelerationXZ = sqrt(acceleration.X * acceleration.X + acceleration.Z * acceleration.Z);
    float accelerationYZ = sqrt(acceleration.Y * acceleration.Y + acceleration.Z * acceleration.Z);

    // calculate pitch and roll from accelerometer itself
    float accelerometerPitch = atan2(acceleration.Y, accelerationXZ);
    float accelerometerRoll = atan2(acceleration.X, accelerationYZ);

    // calculate yaw from magnetometer
    float magnetometerYaw = -0.1f * PI * (atan2(magneticField.Z, magneticField.X) + (magneticField.Z >= 0.0f ? -PI : PI));

    // store sensor values for calculation of deviation
    float previousSensorPitch = sensorPitch;
    float previousSensorRoll = sensorRoll;
    float previousSensorYaw = sensorYaw;

    // calculate sensor pitch, roll anfd yaw using complementary filter
    const float SensorFilterFactor = 0.02f;
    sensorPitch = (1.0f - SensorFilterFactor) * (sensorPitch + angularRate.Y * deltaT) + SensorFilterFactor * accelerometerPitch;
    sensorRoll = (1.0f - SensorFilterFactor) * (sensorRoll + angularRate.X * deltaT) + SensorFilterFactor * accelerometerRoll;
    sensorYaw = (1.0f - SensorFilterFactor) * (sensorYaw + angularRate.Z * deltaT) + SensorFilterFactor * magnetometerYaw;

    // calculate sensor pitch, roll and yaw variability
    const float variabilityFilterFactor = 0.01f;
    float reciprocalDeltaT = (deltaT > 0.0f) ? (1.0f / deltaT) : 1.0f;
    sensorPitchVariability = (1.0f - variabilityFilterFactor) * sensorPitchVariability + variabilityFilterFactor * fabs(sensorPitch - previousSensorPitch) * reciprocalDeltaT;
    sensorRollVariability = (1.0f - variabilityFilterFactor) * sensorRollVariability + variabilityFilterFactor * fabs(sensorRoll - previousSensorRoll) * reciprocalDeltaT;
    sensorYawVariability = (1.0f - variabilityFilterFactor) * sensorYawVariability + variabilityFilterFactor * fabs(sensorYaw - previousSensorYaw) * reciprocalDeltaT;

    // store sensor pitch, roll and yaw reference values
    const float sensorVariabilityThreshold = 0.0025f;
    if((sensorPitchVariability < sensorVariabilityThreshold) &&
       (sensorRollVariability < sensorVariabilityThreshold) &&
       (sensorYawVariability < sensorVariabilityThreshold))
    {
        sensorPitchReference = sensorPitch;
        sensorRollReference = sensorRoll;
        sensorYawReference = sensorYaw;
        calibrationLed = 1;
    }
    else
    {
        calibrationLed = 0;
    }

    // calculate sensor calibrated values
    float calibratedSensorPitch = sensorPitch - sensorPitchReference;
    float calibratedSensorRoll = sensorRoll - sensorRollReference;
    float calibratedSensorYaw = sensorYaw - sensorYawReference;

    //XXX test
    g_gyroX = angularRate.X; g_gyroY = angularRate.Y; g_gyroZ = angularRate.Z;
    g_accX = acceleration.X; g_accY = acceleration.Y; g_accZ = acceleration.Z;
    g_magX = magneticField.X; g_magY = magneticField.Y; g_magZ = magneticField.Z;
    g_sensorPitch = sensorPitch;
    g_sensorRoll = sensorRoll;
    g_sensorYaw = sensorYaw;

    // calculate joystick pitch and roll depending on the joystick yaw
    float sin2yaw = sin(sensorYaw) * fabs(sin(sensorYaw));
    float cos2yaw = cos(sensorYaw) * fabs(cos(sensorYaw));

    float joystickPitch = calibratedSensorPitch * cos2yaw + calibratedSensorRoll * sin2yaw;
    float joystickRoll = calibratedSensorRoll * cos2yaw - calibratedSensorPitch * sin2yaw;
    float joystickYaw = calibratedSensorYaw;

    // calculate joystick axes gain
    //joystickGainFilter.calculate(joystickGainPotentiometer.read() + 0.5f);  // range 0.5 .. 1.5
    joystickGainFilter.calculate(1.0f);  // XXX temporary

    float leftBrake;
    float rightBrake;

    // scale joystick axes to USB joystick report range
    if(brakeActive)
    {
        // both brakes from joystick deflected forward
        leftBrake = rightBrake = ( joystickPitch < 0 ? -joystickPitch : 0.0f);
        // left and right brakes from joystick deflected sideways
        leftBrake += (joystickRoll < 0 ? -joystickRoll : 0.0f);
        rightBrake += (joystickRoll > 0 ? joystickRoll : 0.0f);
    }
    else
    {
        // update pitch axis when not braking only
        joystickData.Y = scale<float, int16_t>(-0.9f, 0.9f, joystickPitch * joystickGainFilter.getValue(), -32767, 32767);
        // release brakes
        leftBrake = 0.0f;
        rightBrake = 0.0f;
    }
    joystickData.X = scale<float, int16_t>(-1.45f, 1.45f, joystickRoll * joystickGainFilter.getValue(), -32767, 32767);
    joystickData.Z = scale<float, int16_t>(-0.78f, 0.78f, joystickYaw * joystickGainFilter.getValue(), -32767, 32767);

    joystickData.slider = scale<float, int16_t>(0.0f, 1.0f, throttlePotentiometer.read(), -32767, 32767);
    joystickData.dial = scale<float, int16_t>(0.0f, 1.0f, propellerPotentiometer.read(), -32767, 32767);
    joystickData.wheel = scale<float, int16_t>(0.0f, 1.0f, mixturePotentiometer.read(), -32767, 32767);

    joystickData.Rx = scale<float, int16_t>(0.0f, 1.0f, leftBrake, -32767, 32767);
    joystickData.Ry = scale<float, int16_t>(0.0f, 1.0f, rightBrake, -32767, 32767);

    // set joystick buttons
    setJoystickButtons();

    // set joystick hat
    setJoystickHat();

    usbJoystick.sendReport(joystickData);

    // LED heartbeat
    systemLed = ((counter & 0x68) == 0x68);
}

/*
 * display status of FlightControl
 */
void Yoke::displayStatus(CommandVector cv)
{
    printf("yoke mode = %s\r\n", modeTexts[static_cast<int>(yokeMode)].c_str());
    printf("IMU sensor pitch = %f\r\n", sensorPitch);
    printf("IMU sensor roll = %f\r\n", sensorRoll);
    printf("IMU sensor yaw = %f\r\n", sensorYaw);
    printf("joystick X = %d\r\n", joystickData.X);
    printf("joystick Y = %d\r\n", joystickData.Y);
    printf("joystick Z = %d\r\n", joystickData.Z);
    printf("joystick Rx = %d\r\n", joystickData.Rx);
    printf("joystick Ry = %d\r\n", joystickData.Ry);
    printf("joystick Rz = %d\r\n", joystickData.Rz);
    printf("joystick slider = %d\r\n", joystickData.slider);
    printf("joystick dial = %d\r\n", joystickData.dial);
    printf("joystick wheel = %d\r\n", joystickData.wheel);
    printf("joystick hat = 0x%02X\r\n", joystickData.hat);
    printf("joystick buttons = 0x%04X\r\n", joystickData.buttons);
}

/*
set joystick buttons
*/
void Yoke::setJoystickButtons(void)
{
    auto setButton = [&](uint8_t input, uint8_t position)
    {
        if(input)
        {
            joystickData.buttons &= ~(1 << position);
        }
        else
        {
            joystickData.buttons |= 1 << position;
        }
    };

    setButton(flapsUpSwitch.read(), 0);
    setButton(flapsDownSwitch.read(), 1);
    setButton(gearUpSwitch.read(), 2);
    setButton(gearDownSwitch.read(), 3);
    setButton(redPushbutton.read(), 4);
    setButton(greenPushbutton.read(), 5);

    if(hatMode == HatSwitchMode::TrimMode)
    {
        uint8_t hatPosition = hatSwitch.getPosition();
        setButton(hatPosition != 1, 6); // elevator trim down
        setButton(hatPosition != 5, 7); // elevator trim up
        setButton(hatPosition != 3, 8); // yaw trim right
        setButton(hatPosition != 7, 9); // yaw trim left
    }
    else
    {
        // in hat mode trim buttons always off
        joystickData.buttons &= ~(0x000F << 6);
    }

    setButton(hatCenterSwitch.read(), 10);
    setButton(setSwitch.read(), 11);
    setButton(resetSwitch.read(), 12);
    setButton(reverserSwitch.read(), 13);
}

/*
set joystick HAT
*/
void Yoke::setJoystickHat(void)
{
    if(hatMode == HatSwitchMode::HatMode)
    {
        joystickData.hat = hatSwitch.getPosition();
    }
    else
    {
        // in trim mode hat position is neutral
        joystickData.hat = 0;
    }
}

/*
* display yoke mode on screen
*/
void Yoke::displayMode(void)
{
    Display::getInstance().setFont(FontTahoma11, false, 127);
    std::string text = "mode: " + modeTexts[static_cast<int>(yokeMode)]; 
    Display::getInstance().print(0, 2, text);
    Display::getInstance().update();
}