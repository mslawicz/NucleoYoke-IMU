#include "Yoke.h"
#include "Alarm.h"
#include "Convert.h"
#include "Display.h"
#include "Logger.h"
#include "Menu.h"
#include "Storage.h"
#include <iomanip>

//XXX global variables for test
float g_gyroX, g_gyroY, g_gyroZ;    //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
float g_accX, g_accY, g_accZ;       //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
float g_magX, g_magY, g_magZ;       //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
float g_sensorPitch, g_sensorRoll, g_sensorYaw;     //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED2),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    imuInterruptSignal(LSM9DS1_INT1, PullDown),
    i2cBus(I2C2_SDA, I2C2_SCL),
    sensorGA(i2cBus, LSM9DS1_AG_ADD),
    sensorM(i2cBus, LSM9DS1_M_ADD),
    calibrationLed(LED1, 0),
    flapsUpSwitch(PE_3, PullUp),
    flapsDownSwitch(PE_4, PullUp),
    gearUpSwitch(PF_4, PullUp),
    gearDownSwitch(PF_5, PullUp),
    redPushbutton(PG_0, PullUp),
    greenPushbutton(PB_2, PullUp),
    hatCenterSwitch(PG_15, PullUp),
    setSwitch(PE_1, PullUp),
    resetSwitch(PE_6, PullUp),
    reverserSwitch(PG_3, PullUp),
    speedBrakeSwitch(PG_2, PullUp),
    hatModeToggle(PF_9, PullUp),
    viewModeToggle(PF_8, PullUp),
    hatModeShift(PB_13, PullUp),
    throttlePotentiometer(PA_0),
    propellerPotentiometer(PA_4),
    mixturePotentiometer(PA_1),
    orangePotentiometer(PC_0),
    yellowPotentiometer(PC_1),
    blueGrayPotentiometer(PA_5),
    hatSwitch(PG_13, PG_9, PG_12, PG_10),
    joystickGainFilter(0.01F)       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
{
    LOG_INFO("Yoke object created");

    constexpr int I2CBusFreq = 400000;
    i2cBus.frequency(I2CBusFreq);

    // connect USB joystick
    usbJoystick.connect();

    // configure IMU sensor
    // Gyroscope ODR=119 Hz, full scale 500 dps
    // int/out selection default
    // low power disable, HPF enable, HPF=0.05 Hz
    sensorGA.write(static_cast<uint8_t>(LSM9DS1reg::CTRL_REG1_G), std::vector<uint8_t>{0x68, 0x00, 0x47});      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    // Accelerometer ODR=119 Hz, full scale +=2g
    sensorGA.write(static_cast<uint8_t>(LSM9DS1reg::CTRL_REG6_XL), std::vector<uint8_t>{0x60});     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    // INT1<-DRDY_G
    sensorGA.write(static_cast<uint8_t>(LSM9DS1reg::INT1_CTRL), std::vector<uint8_t>{0x02});        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // configure magnetometer sensor
    // Magnetometer X&Y high-performance mode, ODR=80 Hz
    // full scale +-16 gauss
    // continues conversion mode
    // Z-axis high-performance mode
    sensorM.write(static_cast<uint8_t>(LSM9DS1reg::CTRL_REG1_M), std::vector<uint8_t>{0x5C, 0x60, 0x00, 0x80});     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // restore yoke parameters
    yokeMode = KvStore::getInstance().restore<YokeMode>("/kv/yokeMode", YokeMode::FixedWing,
        static_cast<YokeMode>(0),
        static_cast<YokeMode>(static_cast<int>(YokeMode::Size) - 1));

    throttleInputMin = KvStore::getInstance().restore<float>("/kv/throttleInputMin", 0.0F, 0.0F, 0.49F);        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    throttleInputMax = KvStore::getInstance().restore<float>("/kv/throttleInputMax", 1.0F, 0.51F, 1.0F);        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    sensorPitchReference = KvStore::getInstance().restore<float>("/kv/sensorPitchRef", 0.0F, -0.5F, 0.5F);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    sensorRollReference = KvStore::getInstance().restore<float>("/kv/sensorRollRef", 0.0F, -0.5F, 0.5F);        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    sensorYawReference = KvStore::getInstance().restore<float>("/kv/sensorYawRef", 0.0F, -0.5F, 0.5F);          //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // call handler on IMU interrupt rise signal
    imuInterruptSignal.rise(callback(this, &Yoke::imuInterruptHandler));
    // this timeout calls handler for the first time
    // next calls will be executed upon IMU INT1 interrupt signal
    constexpr std::chrono::milliseconds HandlerDelay = 100ms;
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), HandlerDelay);

    // start handler timer
    handlerTimer.start();

    // register console commands
    Console::getInstance().registerCommand("ys", "display yoke status", callback(this, &Yoke::displayStatus));

    // add menu items
    Menu::getInstance().addItem("calibrate", callback(this, &Yoke::toggleAxisCalibration));
    Menu::getInstance().addItem("stopwatch", callback(this, &Yoke::toggleStopwatch));
}



/*
* yoke handler is called on IMU interrupt signal
*/
void Yoke::handler()
{
    // this timeout is set only for the case of lost IMU interrupt signal
    // the timeout should never happen, as the next interrupt should be called earlier
    constexpr std::chrono::milliseconds NoIntTimeout = 20ms;
    imuIntTimeout.attach(callback(this, &Yoke::imuInterruptHandler), NoIntTimeout);

    // measure time elapsed since the previous call [s]
    float deltaT = std::chrono::duration<float>(handlerTimer.elapsed_time()).count();
    handlerTimer.reset();
    counter++;

    // set HAT switch mode
    if((hatModeToggle.read() ^ hatModeShift.read()) == 0)       //NOLINT(hicpp-signed-bitwise)
    {
        // HAT mode shift pressed in HAT view mode or
        // HAT mode shift NOT pressed in HAT trim mode
        hatMode = HatSwitchMode::TrimMode;
    }
    else if(viewModeToggle.read() != 0)      // hat view mode toggle down
    {
        hatMode = HatSwitchMode::QuickViewMode;
    }
    else        // hat view mode toggle up
    {
        hatMode = HatSwitchMode::FreeViewMode;
    }

    // set brake mode from RESET pushbutton
    bool brakeActive = (resetSwitch.read() == 0);

    std::vector<uint8_t> sensorData;
    if(imuInterruptSignal.read() == 1)
    {
        // interrupt signal is active
        // read IMU sensor data
        sensorData = sensorGA.read(static_cast<uint8_t>(LSM9DS1reg::OUT_X_L_G), 12);    //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        gyroscopeData.X = *reinterpret_cast<int16_t*>(&sensorData[4]);
        gyroscopeData.Y = *reinterpret_cast<int16_t*>(&sensorData[2]);
        gyroscopeData.Z = *reinterpret_cast<int16_t*>(&sensorData[0]);
        accelerometerData.X = *reinterpret_cast<int16_t*>(&sensorData[10]);     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        accelerometerData.Y = *reinterpret_cast<int16_t*>(&sensorData[8]);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        accelerometerData.Z = *reinterpret_cast<int16_t*>(&sensorData[6]);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

        // read magnetometer data
        sensorData = sensorM.read(static_cast<uint8_t>(LSM9DS1reg::OUT_X_L_M), 6);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
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
    angularRate.X = -AngularRateResolution * static_cast<float>(gyroscopeData.Y);
    angularRate.Y = -AngularRateResolution * static_cast<float>(gyroscopeData.Z);
    angularRate.Z = AngularRateResolution * static_cast<float>(gyroscopeData.X);
    // acceleration in g
    acceleration.X = AccelerationResolution * static_cast<float>(accelerometerData.Z);
    acceleration.Y = -AccelerationResolution * static_cast<float>(accelerometerData.Y);
    acceleration.Z = -AccelerationResolution * static_cast<float>(accelerometerData.X);
    // magnetic field in gauss
    magneticField.X = MagneticFieldResolution * static_cast<float>(magnetometerData.X);
    magneticField.Y = MagneticFieldResolution * static_cast<float>(magnetometerData.Y);
    magneticField.Z = MagneticFieldResolution * static_cast<float>(magnetometerData.Z);

    float accelerationXZ = sqrt(acceleration.X * acceleration.X + acceleration.Z * acceleration.Z);
    float accelerationYZ = sqrt(acceleration.Y * acceleration.Y + acceleration.Z * acceleration.Z);

    // calculate pitch and roll from accelerometer itself [rad]
    float accelerometerPitch = atan2(acceleration.Y, accelerationXZ);
    float accelerometerRoll = atan2(acceleration.X, accelerationYZ);

    // calculate yaw from magnetometer  [rad]
    constexpr float MagnetometerYawGain = -0.1F;
    float magnetometerYaw = MagnetometerYawGain * PI * (atan2(magneticField.Z, magneticField.X) + (magneticField.Z >= 0.0F ? -PI : PI));

    // store sensor values for calculation of deviation
    float previousSensorPitch = sensorPitch;
    float previousSensorRoll = sensorRoll;
    float previousSensorYaw = sensorYaw;

    // calculate sensor pitch, roll and yaw using complementary filter [rad]
    const float SensorFilterFactor = 0.02F;
    sensorPitch = (1.0F - SensorFilterFactor) * (sensorPitch + angularRate.Y * deltaT) + SensorFilterFactor * accelerometerPitch;
    sensorRoll = (1.0F - SensorFilterFactor) * (sensorRoll + angularRate.X * deltaT) + SensorFilterFactor * accelerometerRoll;
    sensorYaw = (1.0F - SensorFilterFactor) * (sensorYaw + angularRate.Z * deltaT) + SensorFilterFactor * magnetometerYaw;

    // calculate sensor pitch, roll and yaw variability
    const float VariabilityFilterFactor = 0.01F;
    float reciprocalDeltaT = (deltaT > 0.0F) ? (1.0F / deltaT) : 1.0F;
    sensorPitchVariability = (1.0F - VariabilityFilterFactor) * sensorPitchVariability + VariabilityFilterFactor * fabs(sensorPitch - previousSensorPitch) * reciprocalDeltaT;
    sensorRollVariability = (1.0F - VariabilityFilterFactor) * sensorRollVariability + VariabilityFilterFactor * fabs(sensorRoll - previousSensorRoll) * reciprocalDeltaT;
    sensorYawVariability = (1.0F - VariabilityFilterFactor) * sensorYawVariability + VariabilityFilterFactor * fabs(sensorYaw - previousSensorYaw) * reciprocalDeltaT;

    // store sensor pitch, roll and yaw reference values
    constexpr float SensorVariabilityThreshold = 0.0025F;
    if((sensorPitchVariability < SensorVariabilityThreshold) &&
       (sensorRollVariability < SensorVariabilityThreshold) &&
       (sensorYawVariability < SensorVariabilityThreshold))
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

    // calculate sensor calibrated values [rad]
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

    // calculate joystick pitch and roll depending on the joystick yaw [rad]
    float sin2yaw = sin(sensorYaw) * fabs(sin(sensorYaw));
    float cos2yaw = cos(sensorYaw) * fabs(cos(sensorYaw));

    float joystickPitch = calibratedSensorPitch * cos2yaw + calibratedSensorRoll * sin2yaw;
    float joystickRoll = calibratedSensorRoll * cos2yaw - calibratedSensorPitch * sin2yaw;
    float joystickYaw = calibratedSensorYaw;

    // calculate joystick axes gain
    joystickGainFilter.calculate(blueGrayPotentiometer.read() + 0.5F);  // range 0.5 .. 1.5     NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    float leftBrake{0.0F};
    float rightBrake{0.0F};

    // scale joystick axes to USB joystick report range
    if(brakeActive)
    {
        // both brakes from joystick deflected forward
        leftBrake = rightBrake = 2.0F * ( joystickPitch < 0 ? -joystickPitch : 0.0F);       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        // left and right brakes from joystick deflected sideways
        leftBrake += (joystickRoll < 0 ? -joystickRoll : 0.0F);
        rightBrake += (joystickRoll > 0 ? joystickRoll : 0.0F);
    }
    else
    {
        // update pitch axis when not braking only
        constexpr float MaxAngleY = 0.9F;   // [rad]
        joystickData.Y = scale<float, int16_t>(-MaxAngleY, MaxAngleY, joystickPitch * joystickGainFilter.getValue(), -Max15bit, Max15bit);
        // release brakes
        leftBrake = 0.0F;
        rightBrake = 0.0F;
    }
    constexpr float MaxAngleX = 1.45F;   // [rad]
    joystickData.X = scale<float, int16_t>(-MaxAngleX, MaxAngleX, joystickRoll * joystickGainFilter.getValue(), -Max15bit, Max15bit);
    constexpr float MaxAngleZ = 0.78F;   // [rad]
    joystickData.Rz = scale<float, int16_t>(-MaxAngleZ, MaxAngleZ, joystickYaw * joystickGainFilter.getValue(), -Max15bit, Max15bit);

    throttleFilter.calculate(throttlePotentiometer.read());
    throttleInput = throttleFilter.getValue();
    const float ThrottleDeadZone = 0.03F;
    joystickData.slider = scale<float, int16_t>(throttleInputMin + ThrottleDeadZone, throttleInputMax - ThrottleDeadZone, throttleInput, 0, Max15bit);

    propellerFilter.calculate(propellerPotentiometer.read());
    joystickData.dial = scale<float, int16_t>(0.0F, 1.0F, propellerFilter.getValue(), 0, Max15bit);

    mixtureFilter.calculate(mixturePotentiometer.read());
    joystickData.Z = scale<float, int16_t>(0.0F, 1.0F, mixtureFilter.getValue(), -Max15bit, Max15bit);
    joystickData.Rx = scale<float, int16_t>(0.0F, 1.0F, leftBrake, 0, Max15bit);
    joystickData.Ry = scale<float, int16_t>(0.0F, 1.0F, rightBrake, 0, Max15bit);

    // set joystick buttons
    setJoystickButtons();

    usbJoystick.sendReport(joystickData);

    // analog axis calibration on user request
    axisCalibration();

    // LED heartbeat
    constexpr uint32_t HeartbeatMask = 0x68U;
    systemLed = (counter & HeartbeatMask) == HeartbeatMask ? 1U : 0;
}

/*
 * display status of FlightControl
 */
void Yoke::displayStatus(CommandVector&  /*cv*/)
{
    std::cout << "yoke mode = " << modeTexts[static_cast<int>(yokeMode)] << std::endl;
    std::cout << "IMU sensor pitch/roll/yaw = " << sensorPitch << ", " << sensorRoll << ", " << sensorYaw << std::endl;
    std::cout << "reference pitch/roll/yaw = " << sensorPitchReference << ", " << sensorRollReference << ", " << sensorYawReference << std::endl;
    std::cout << "joystick X = " << joystickData.X << std::endl;
    std::cout << "joystick Y = " << joystickData.Y << std::endl;
    std::cout << "joystick Z = " << joystickData.Z << std::endl;
    std::cout << "joystick Rx = " << joystickData.Rx << std::endl;
    std::cout << "joystick Ry = " << joystickData.Ry << std::endl;
    std::cout << "joystick Rz = " << joystickData.Rz << std::endl;
    std::cout << "joystick slider = " << joystickData.slider << std::endl;
    std::cout << "joystick dial = " << joystickData.dial << std::endl;
    std::cout << "joystick hat = 0x" << std::hex << std::setw(2) << std::setfill('0') << joystickData.hat << std::endl;
    std::cout << "joystick buttons = 0x" << std::hex << std::setw(8) << std::setfill('0') << joystickData.buttons << std::endl;     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    std::cout << "throttle min/value/max = " << throttleInputMin << ", " << throttleInput << ", " << throttleInputMax << std::endl;
}

/*
set joystick buttons
*/
void Yoke::setJoystickButtons()
{
    auto setButton = [&](uint8_t input, uint8_t position)
    {
        if(input != 0)
        {
            joystickData.buttons &= ~(1 << position);       //NOLINT(hicpp-signed-bitwise)
        }
        else
        {
            joystickData.buttons |= 1 << position;          //NOLINT(hicpp-signed-bitwise)
        }
    };

    joystickData.buttons = 0;

    setButton(flapsUpSwitch.read(), 0);
    setButton(flapsDownSwitch.read(), 1);
    setButton(gearUpSwitch.read(), 2);
    setButton(gearDownSwitch.read(), 3);
    setButton(redPushbutton.read(), 4);
    setButton(greenPushbutton.read(), 5);       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // set buttons from HAT switch
    uint8_t hatPosition = hatSwitch.getPosition();

    switch(hatMode)
    {
        case HatSwitchMode::FreeViewMode:
            joystickData.hat = hatSwitch.getPosition();
            setButton(hatCenterSwitch.read(), 10);          //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            break;
        case HatSwitchMode::QuickViewMode:
            joystickData.hat = 0;
            if(hatPosition != 0)
            {
                setButton(0, 15 + hatPosition); // set one button in the range 16-23    NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            }
            setButton(hatCenterSwitch.read(), 24);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            break;
        case HatSwitchMode::TrimMode:
            joystickData.hat = 0;
            setButton(static_cast<uint8_t>(hatPosition != 1), 6); // elevator trim down     NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            setButton(static_cast<uint8_t>(hatPosition != 5), 7); // elevator trim up       NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            setButton(static_cast<uint8_t>(hatPosition != 3), 8); // rudder trim right      NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            setButton(static_cast<uint8_t>(hatPosition != 7), 9); // rudder trim left       NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            break;
        default:
            break;
    }

    setButton(setSwitch.read(), 11);        //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    setButton(resetSwitch.read(), 12);      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    setButton(reverserSwitch.read(), 13);   //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    setButton(speedBrakeSwitch.read(), 14); //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}

/*
* display yoke mode on screen
*/
void Yoke::displayMode()
{
    constexpr uint8_t LimitX = 127U;
    Display::getInstance().setFont(static_cast<const uint8_t*>(FontTahoma11), false, LimitX);
    std::string text = "mode: " + modeTexts[static_cast<int>(yokeMode)]; 
    Display::getInstance().print(0, 2, text);
    Display::getInstance().update();
}

/*
analog axis calibration on user request
*/
void Yoke::axisCalibration()
{
    if(isCalibrationOn)
    {
        if(throttleInput < throttleInputMin)
        {
            throttleInputMin = throttleInput;
        }

        if(throttleInput > throttleInputMax)
        {
            throttleInputMax = throttleInput;
        }
    }
}

/*
start / stop axis calibration
*/
void Yoke::toggleAxisCalibration()
{
    if(isCalibrationOn)
    {
        isCalibrationOn = false;
        std::cout << "Axis calibration completed" << std::endl;
        Menu::getInstance().displayMessage("cal. completed", 10);       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        Menu::getInstance().enableMenuChange();

        KvStore::getInstance().store<float>("/kv/throttleInputMin", throttleInputMin);
        KvStore::getInstance().store<float>("/kv/throttleInputMax", throttleInputMax);
        KvStore::getInstance().store<float>("/kv/sensorPitchRef", sensorPitchReference);
        KvStore::getInstance().store<float>("/kv/sensorRollRef", sensorRollReference);
        KvStore::getInstance().store<float>("/kv/sensorYawRef", sensorYawReference);
    }
    else
    {
        isCalibrationOn = true;
        std::cout << "Axis calibration on; move throttle lever from min to max" << std::endl;
        Menu::getInstance().displayMessage("cal. started");
        throttleInputMin = 0.49F;       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        throttleInputMax = 0.51F;       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        Menu::getInstance().disableMenuChange();
    }
}

/*
start / stop pilots stopwatch on display
*/
void Yoke::toggleStopwatch()
{
    if(isStopwatchDisplayed)
    {
        if(chrono::duration_cast<chrono::seconds>(stopwatch.elapsed_time()).count() < 3)
        {
            isStopwatchDisplayed = false;
            stopwatch.stop();
            stopwatchTicker.detach();
            Menu::getInstance().enableDisplay();
            Display::getInstance().clear();
            displayAll();
            Menu::getInstance().enableMenuChange();
        }
        else
        {
            stopwatch.reset();
            displayStopwatch();
        }
    }
    else
    {
        isStopwatchDisplayed = true;
        stopwatch.reset();
        stopwatch.start();
        displayStopwatch();
        constexpr uint32_t MicroInSec = 1000000U;
        stopwatchTicker.attach(callback(this, &Yoke::displayStopwatch), std::chrono::microseconds(MicroInSec));
        Menu::getInstance().disableMenuChange();
        Menu::getInstance().disableDisplay();
    }
}

/*
displays stopwatch on display
*/
void Yoke::displayStopwatch()
{
    constexpr uint16_t SecInMin = 60U;
    constexpr uint8_t refX = 96;
    constexpr uint8_t refY = 33;
    constexpr uint8_t radius = 31;
    uint16_t secondsElapsed = static_cast<uint16_t>(chrono::duration_cast<chrono::seconds>(stopwatch.elapsed_time()).count());
    uint8_t seconds = secondsElapsed % SecInMin;
    uint16_t minutes = secondsElapsed / SecInMin;
    if(seconds == 0)
    {
        char stopwatchString[3];        //NOLINT(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
        Display::getInstance().clear();
        Display::getInstance().setFont(static_cast<const uint8_t*>(FontArial42d));
        sprintf(static_cast<char*>(stopwatchString), "%2d", minutes % 100);     //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        Display::getInstance().print(0, 16, std::string(static_cast<char*>(stopwatchString)));      //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    }
    float sinX = sin(PI * static_cast<float>(seconds) / 30.0F); //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    float cosX = cos(PI * static_cast<float>(seconds) / 30.0F); //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    uint8_t fromX = refX + static_cast<uint8_t>(sinX * radius);
    uint8_t fromY = refY - static_cast<uint8_t>(cosX * radius);
    constexpr uint8_t ShortBar = 5U; 
    uint8_t toRadius = radius - ShortBar;
    if(seconds % ShortBar == 0)
    {
        toRadius -= ShortBar;
    }
    if(seconds % 15 == 0)       //NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    {
        toRadius -= ShortBar;
    }
    uint8_t toX = refX + static_cast<uint8_t>(sinX * static_cast<float>(toRadius));
    uint8_t toY = refY - static_cast<uint8_t>(cosX * static_cast<float>(toRadius));
    Display::getInstance().drawLine(fromX, toX, fromY, toY);
    Display::getInstance().update();
}

/*
display all regular fields
*/
void Yoke::displayAll()
{
    Alarm::getInstance().displayOnScreen();
    displayMode();
    Menu::getInstance().displayItemText();
}