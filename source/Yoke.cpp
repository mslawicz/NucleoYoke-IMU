#include "Yoke.h"

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED1),
    usbJoystick(USB_VID, USB_PID, USB_VER),
    imuInterruptSignal(LSM6DS3_INT1),
    i2cBus(I2C1_SDA, I2C1_SCL),
    sensorGA(i2cBus, LSM6DS3_AG_ADD)
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

    // start handler timer
    handlerTimer.start();
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

    // read IMU sensor data
    auto sensorData = sensorGA.read((uint8_t)LSM6DS3reg::OUT_X_L_G, 12);

    // joystick report test
    JoystickData testData;
    int16_t i16 = (int16_t)(30000 * sin(counter / 100.0f));
    testData.X = i16;
    testData.Y = i16;
    testData.Z = i16;
    testData.Rx = i16;
    testData.Ry= i16;
    testData.Rz = i16;
    testData.slider = i16;
    testData.dial = i16;
    testData.wheel = i16;

    uint8_t phase = (counter / 100) % 9;
    testData.hat = phase;

    phase = (counter / 20) % 16;
    testData.buttons = (1 << phase);

    usbJoystick.sendReport(testData);

    // LED heartbeat
    systemLed = ((counter & 0x68) == 0x68);
}