#include "Yoke.h"
#include "Scale.h"

//XXX global variables for test
float g_gyroX, g_gyroY, g_gyroZ;
float g_accX, g_accY, g_accZ;
float g_sensorPitch, g_sensorRoll, g_sensorYaw;

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED2),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    imuInterruptSignal(LSM6DS3_INT1),
    i2cBus(I2C2_SDA, I2C2_SCL),
    sensorGA(i2cBus, LSM6DS3_AG_ADD),
    calibrationLed(LED1, 0),
    flapsUpSwitch(PB_15, PullUp),
    flapsDownSwitch(PB_13, PullUp),
    gearUpSwitch(PF_4, PullUp),
    gearDownSwitch(PF_5, PullUp),
    redPushbutton(PB_11, PullUp),
    greenPushbutton(PB_2, PullUp),
    hatCenterSwitch(PG_15, PullUp),
    setSwitch(PE_1, PullUp),
    resetSwitch(PE_6, PullUp),
    leftToggle(PG_5, PullUp),
    rightToggle(PG_8, PullUp),
    throttlePotentiometer(PC_5),
    propellerPotentiometer(PC_4),
    mixturePotentiometer(PB_1),
    joystickGainPotentiometer(PA_2),
    tinyJoystickX(PC_3),
    tinyJoystickY(PC_2),
    hatSwitch(PG_13, PG_9, PG_12, PG_10),
    joystickGainFilter(0.01f),
    tensometerThread(osPriorityBelowNormal),
    leftPedalTensometer(PD_0, PD_1, tensometerQueue),
    rightPedalTensometer(PF_8, PF_9, tensometerQueue)
{
    printf("Yoke object created\r\n");

    i2cBus.frequency(400000);

    // connect USB joystick
    usbJoystick.connect();

    // configure IMU sensor
    // INT1<-DRDY_G
    sensorGA.write((uint8_t)LSM6DS3reg::INT1_CTRL, std::vector<uint8_t>{0x02});
    // accelerometer ODR=104 Hz, full scale 2g, antialiasing 400 Hz
    // gyroscope ODR=104 Hz, full scale 500 dps,
    sensorGA.write((uint8_t)LSM6DS3reg::CTRL1_XL, std::vector<uint8_t>{0x40, 0x44});
    // gyroscope HPF enable, HPF=0.0081 Hz
    sensorGA.write((uint8_t)LSM6DS3reg::CTRL7_G, std::vector<uint8_t>{0x40});

    // call handler on IMU interrupt rise signal
    imuInterruptSignal.rise(callback(this, &Yoke::imuInterruptHandler));
    // this timeout calls handler for the first time
    // next calls will be executed upon IMU INT1 interrupt signal
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 0.1f);

    // tensometer queue will be dispatched in another thread
    tensometerThread.start(callback(&tensometerQueue, &EventQueue::dispatch_forever));

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
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 0.02f);

    // measure time elapsed since the previous call
    float deltaT = handlerTimer.read();
    handlerTimer.reset();
    counter++;

    // set HAT switch mode
    hatMode = leftToggle.read() ? HatSwitchMode::TrimMode : HatSwitchMode::HatMode;

    // read IMU sensor data
    auto sensorData = sensorGA.read((uint8_t)LSM6DS3reg::OUT_X_L_G, 12);
    gyroscopeData.X = *reinterpret_cast<int16_t*>(&sensorData[4]);
    gyroscopeData.Y = *reinterpret_cast<int16_t*>(&sensorData[2]);
    gyroscopeData.Z = *reinterpret_cast<int16_t*>(&sensorData[0]);
    accelerometerData.X = *reinterpret_cast<int16_t*>(&sensorData[10]);
    accelerometerData.Y = *reinterpret_cast<int16_t*>(&sensorData[8]);
    accelerometerData.Z = *reinterpret_cast<int16_t*>(&sensorData[6]);

    // calculate IMU sensor physical values; using right hand rule
    // X = roll axis = pointing North
    // Y = pitch axis = pointing East
    // Z = yaw axis = pointing down
    // angular rate in rad/s
    angularRate.X = AngularRateResolution * gyroscopeData.X;
    angularRate.Y = AngularRateResolution * gyroscopeData.Y;
    angularRate.Z = -AngularRateResolution * gyroscopeData.Z;
    // acceleration in g
    acceleration.X = AccelerationResolution * accelerometerData.X;
    acceleration.Y = -AccelerationResolution * accelerometerData.Y;
    acceleration.Z = -AccelerationResolution * accelerometerData.Z;

    float accelerationXZ = sqrt(acceleration.X * acceleration.X + acceleration.Z * acceleration.Z);
    float accelerationYZ = sqrt(acceleration.Y * acceleration.Y + acceleration.Z * acceleration.Z);

    // calculate pitch and roll from accelerometer itself
    float accelerometerPitch = atan2(acceleration.X, accelerationYZ);
    float accelerometerRoll = atan2(acceleration.Y, accelerationXZ);

    // store sensor values for calculation of deviation
    float previousSensorPitch = sensorPitch;
    float previousSensorRoll = sensorRoll;

    // calculate sensor pitch and roll using complementary filter
    const float SensorFilterFactor = 0.02f;
    sensorPitch = (1.0f - SensorFilterFactor) * (sensorPitch + angularRate.Y * deltaT) + SensorFilterFactor * accelerometerPitch;
    sensorRoll = (1.0f - SensorFilterFactor) * (sensorRoll + angularRate.X * deltaT) + SensorFilterFactor * accelerometerRoll;

    // calculate sensor pitch and roll variability
    const float variabilityFilterFactor = 0.01f;
    float reciprocalDeltaT = (deltaT > 0.0f) ? (1.0f / deltaT) : 1.0f;
    sensorPitchVariability = (1.0f - variabilityFilterFactor) * sensorPitchVariability + variabilityFilterFactor * fabs(sensorPitch - previousSensorPitch) * reciprocalDeltaT;
    sensorRollVariability = (1.0f - variabilityFilterFactor) * sensorRollVariability + variabilityFilterFactor * fabs(sensorRoll - previousSensorRoll) * reciprocalDeltaT;

    // store sensor pitch and roll reference value
    const float sensorVariabilityThreshold = 0.0025f;
    if((sensorPitchVariability < sensorVariabilityThreshold) &&
       (sensorRollVariability < sensorVariabilityThreshold))
    {
        sensorPitchReference = sensorPitch;
        sensorRollReference = sensorRoll;
        calibrationLed = 1;
    }
    else
    {
        calibrationLed = 0;
    }

    // calculate sensor calibrated values
    float calibratedSensorPitch = sensorPitch - sensorPitchReference;
    float calibratedSensorRoll = sensorRoll - sensorRollReference;

    // calculate sensor relative yaw
    sensorYaw += angularRate.Z * deltaT;

    //XXX test
    g_gyroX = angularRate.X; g_gyroY = angularRate.Y; g_gyroZ = angularRate.Z;
    g_accX = acceleration.X; g_accY = acceleration.Y; g_accZ = acceleration.Z;
    g_sensorPitch = sensorPitch;
    g_sensorRoll = sensorRoll;
    g_sensorYaw = sensorYaw;

    // autocalibration of yaw
    const float YawAutocalibrationThreshold = 0.15f;    // joystick deflection threshold for disabling yaw autocalibration function
    const float YawAutocalibrationFactor = 0.9999f;      // yaw autocalibration factor
    float joystickDeflection = sqrt(calibratedSensorPitch * calibratedSensorPitch + calibratedSensorRoll * calibratedSensorRoll);
    if(joystickDeflection < YawAutocalibrationThreshold)
    {
        sensorYaw *= YawAutocalibrationFactor;
    }

    // calculate joystick pitch and roll depending on the joystick yaw
    float sin2yaw = sin(sensorYaw) * fabs(sin(sensorYaw));
    float cos2yaw = cos(sensorYaw) * fabs(cos(sensorYaw));

    float joystickPitch = calibratedSensorPitch * cos2yaw + calibratedSensorRoll * sin2yaw;
    float joystickRoll = calibratedSensorRoll * cos2yaw - calibratedSensorPitch * sin2yaw;

    // calculate joystick axes gain
    joystickGainFilter.calculate(joystickGainPotentiometer.read() + 0.5f);  // range 0.5 .. 1.5

    // scale joystick axes to USB joystick report range
    joystickData.X = scale<float, int16_t>(-1.45f, 1.45f, joystickRoll * joystickGainFilter.getValue(), -32767, 32767);
    joystickData.Y = scale<float, int16_t>(-0.9f, 0.9f, joystickPitch * joystickGainFilter.getValue(), -32767, 32767);
    joystickData.Z = scale<float, int16_t>(-0.78f, 0.78f, sensorYaw * joystickGainFilter.getValue(), -32767, 32767);

    joystickData.slider = scale<float, int16_t>(0.0f, 1.0f, throttlePotentiometer.read(), -32767, 32767);
    joystickData.dial = scale<float, int16_t>(0.0f, 1.0f, propellerPotentiometer.read(), -32767, 32767);
    joystickData.wheel = scale<float, int16_t>(0.0f, 1.0f, mixturePotentiometer.read(), -32767, 32767);

    float leftBrake = 1.0f - tinyJoystickX.read() - tinyJoystickY.read();
    float rightBrake = tinyJoystickY.read() - tinyJoystickX.read();
    joystickData.Rx = scale<float, int16_t>(0.1f, 0.5f, leftBrake, -32767, 32767);
    joystickData.Ry = scale<float, int16_t>(0.1f, 0.5f, rightBrake, -32767, 32767);

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
