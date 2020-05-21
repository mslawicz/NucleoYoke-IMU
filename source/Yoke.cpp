#include "Yoke.h"
#include "Scale.h"

float g_first;
float g_force;
float g_avForce;

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED2),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    calibrationLed(LED1, 0),
    flapsUpSwitch(PB_15, PullUp),
    flapsDownSwitch(PB_13, PullUp),
    gearUpSwitch(PF_4, PullUp),
    gearDownSwitch(PF_5, PullUp),
    redPushbutton(PB_11, PullUp),
    greenPushbutton(PB_2, PullUp),
    throttlePotentiometer(PC_5),
    propellerPotentiometer(PC_4),
    mixturePotentiometer(PB_1),
    tinyJoystickX(PC_3),
    tinyJoystickY(PC_2),
    servo35(PC_9, 0.57e-3, 2.45e-3, 0.5f),
    forceSensor(PD_2, PC_12, eventQueue)
{
    printf("Yoke object created\r\n");

    // connect USB joystick
    usbJoystick.connect();

    // this timeout calls handler for the first time
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
    // call handler after 10 ms
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), 0.01f);

    // measure time elapsed since the previous call
    float deltaT = handlerTimer.read();
    handlerTimer.reset();
    counter++;

    //XXX servo test
    float alpha = 0.1f * propellerPotentiometer.read();
    float g_force = forceSensor.getValue();
    g_avForce = (1.0f - alpha) * g_avForce + alpha * g_force;
    float servoForce = 5.0f * mixturePotentiometer.read() * g_avForce;
    servo35.setValue(0.5f - servoForce);
    g_first = g_force;

    if(counter % 50 == 0)
    {
        printf("a=%4f f=%f av=%f sf=%f\r\n", alpha, g_force, g_avForce, servoForce);
    }

    // read IMU sensor data
    auto sensorData = std::vector<uint8_t>(12, 0);
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