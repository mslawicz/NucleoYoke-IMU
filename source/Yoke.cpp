#include "Yoke.h"

Yoke::Yoke(events::EventQueue& eventQueue) :
    eventQueue(eventQueue),
    systemLed(LED1),
    usbJoystick(USB_VID, USB_PID, USB_VER)
{
    printf("Yoke object created\r\n");

    // connect USB joystick
    usbJoystick.connect();

    //Yoke handler is executed every 10 ms
    eventQueue.call_every(10, callback(this, &Yoke::handler));
}



/*
* yoke handler should be called periodically every 10 ms
*/
void Yoke::handler(void)
{
    counter++;

    // joystick report test
    JoystickData testData;
    int16_t i16 = (int16_t)(30000 * sin(counter / 100.0f));
    testData.X = i16;
    testData.Y = i16;
    testData.Z = i16;
    testData.Rx = i16;
    testData.Ry= i16;
    testData.Rz = i16;

    uint8_t phase = (counter / 100) % 9;
    testData.hat = phase;

    phase = (counter / 20) % 16;
    testData.buttons = (1 << phase);

    usbJoystick.sendReport(testData);

    // LED heartbeat
    systemLed = ((counter & 0x68) == 0x68);
}