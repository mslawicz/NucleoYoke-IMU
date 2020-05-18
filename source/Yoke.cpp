#include "Yoke.h"
#include "Scale.h"

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED2),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    i2cBus(I2C1_SDA, I2C1_SCL),
    stepperMotorController(i2cBus, PCA9685_ADD),
    //stepperMotorController(i2cBus, LSM6DS3_ADD),
    calibrationLed(LED1, 0),
    flapsUpSwitch(PB_15, PullUp),
    flapsDownSwitch(PB_13, PullUp),
    gearUpSwitch(PF_4, PullUp),
    gearDownSwitch(PF_5, PullUp),
    redPushbutton(PB_11, PullUp),
    greenPushbutton(PB_2, PullUp),
    throttlePotentiometer(PC_0),//XXX PC_5),
    propellerPotentiometer(PC_1),//XXX PC_4),
    mixturePotentiometer(PB_1),
    tinyJoystickX(PC_3),
    tinyJoystickY(PC_2)
{
    printf("Yoke object created\r\n");

    i2cBus.frequency(400000);

    // connect USB joystick
    usbJoystick.connect();

    // configure PCA9685
    stepperMotorController.write(0x00, std::vector<uint8_t>{0x30}); //sleep mode
    stepperMotorController.write(0xFE, std::vector<uint8_t>{0x03}); //prescale value for high frequency
    stepperMotorController.write(0x00, std::vector<uint8_t>{0x20, 0x0C}); // normal mode , totem pole, register increment
    auto registers = stepperMotorController.read(0x01, 1);
    printf("PCA9685[0x01]=%u\r\n", registers[0]);

    // switch motor voltage on
    setChannel(2, 1);
    setChannel(7, 1);

    // this timeout calls handler for the first time
    // next calls will be executed upon IMU INT1 interrupt signal
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 0.1f);

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
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 0.008f);

    // measure time elapsed since the previous call
    float deltaT = handlerTimer.read();
    handlerTimer.reset();
    counter++;

    //XXX test
    uint8_t phase;
    phase = (counter / 16) % 4;

    switch(phase)
    {
    case 0:
        setChannel(3, 1);
        setChannel(4, 0);
        setChannel(5, 0);
        setChannel(6, 1);
        break;
    case 1:
        setChannel(3, 1);
        setChannel(4, 0);
        setChannel(5, 1);
        setChannel(6, 0);
        break;
    case 2:
        setChannel(3, 0);
        setChannel(4, 1);
        setChannel(5, 1);
        setChannel(6, 0);
        break;
    case 3:
        setChannel(3, 0);
        setChannel(4, 1);
        setChannel(5, 0);
        setChannel(6, 1);
        break;
    default:
        setChannel(3, 0);
        setChannel(4, 0);
        setChannel(5, 0);
        setChannel(6, 0);
        break;
    }

    setChannel(0, phase == 0);

    // read IMU sensor data
    auto sensorData = std::vector<uint8_t>(12, 0); //sensorGA.read((uint8_t)LSM6DS3reg::OUT_X_L_G, 12);
    gyroscopeData.X = *reinterpret_cast<int16_t*>(&sensorData[0]);
    gyroscopeData.Y = *reinterpret_cast<int16_t*>(&sensorData[2]);
    gyroscopeData.Z = *reinterpret_cast<int16_t*>(&sensorData[4]);
    accelerometerData.X = *reinterpret_cast<int16_t*>(&sensorData[6]);
    accelerometerData.Y = *reinterpret_cast<int16_t*>(&sensorData[8]);
    accelerometerData.Z = *reinterpret_cast<int16_t*>(&sensorData[10]);

    // calculate IMU sensor physical values; using right hand rule
    // X = roll axis = pointing North
    // Y = pitch axis = pointing East
    // Z = yaw axis = pointing down
    // angular rate in rad/s
    angularRate.X = -AngularRateResolution * gyroscopeData.X;
    angularRate.Y = AngularRateResolution * gyroscopeData.Y;
    angularRate.Z = -AngularRateResolution * gyroscopeData.Z;
    // acceleration in g
    acceleration.X = -AccelerationResolution * accelerometerData.X;
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

    // scale joystick axes to USB joystick report range
    joystickData.X = scale<float, int16_t>(-1.5f, 1.5f, joystickRoll, -32767, 32767);
    joystickData.Y = scale<float, int16_t>(-1.0f, 1.0f, joystickPitch, -32767, 32767);
    joystickData.Z = scale<float, int16_t>(-0.75f, 0.75f, sensorYaw, -32767, 32767);

    joystickData.slider = scale<float, int16_t>(0.0f, 1.0f, throttlePotentiometer.read(), -32767, 32767);
    joystickData.dial = scale<float, int16_t>(0.0f, 1.0f, propellerPotentiometer.read(), -32767, 32767);
    joystickData.wheel = scale<float, int16_t>(0.0f, 1.0f, mixturePotentiometer.read(), -32767, 32767);

    float leftBrake = tinyJoystickX.read() - tinyJoystickY.read();
    float rightBrake = 1.0f - tinyJoystickX.read() - tinyJoystickY.read();
    joystickData.Rx = scale<float, int16_t>(0.1f, 0.5f, leftBrake, -32767, 32767);
    joystickData.Ry = scale<float, int16_t>(0.1f, 0.5f, rightBrake, -32767, 32767);

    // set joystick buttons
    setJoystickButtons();

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
    auto setButton = [&](DigitalIn& input, uint8_t position)
    {
        if(input.read())
        {
            joystickData.buttons &= ~(1 << position);
        }
        else
        {
            joystickData.buttons |= 1 << position;
        }
    };

    setButton(flapsUpSwitch, 0);
    setButton(flapsDownSwitch, 1);
    setButton(gearUpSwitch, 2);
    setButton(gearDownSwitch, 3);
    setButton(redPushbutton, 4);
    setButton(greenPushbutton, 5);
}

void Yoke::setChannel(uint8_t chNo, uint8_t value)
{
    static const std::vector<uint8_t> ChannelHigh{0x00, 0x10, 0x00, 0x00};
    static const std::vector<uint8_t> ChannelLow{0x00, 0x00, 0x00, 0x10};
    stepperMotorController.write(6 + chNo * 4, value != 0 ? ChannelHigh : ChannelLow);
}