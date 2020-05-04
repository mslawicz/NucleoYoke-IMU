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

    // LED heartbeat
    systemLed = ((counter & 0x68) == 0x68);
}