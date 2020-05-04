#ifndef YOKE_H_
#define YOKE_H_

#include <mbed.h>

class Yoke
{
public:
    Yoke(events::EventQueue& eventQueue);
private:
    void handler(void);
    events::EventQueue& eventQueue;     // event queue of the main thread
    DigitalOut systemLed;               // yoke heartbeat LED
    uint32_t counter{0};                // counter of handler execution
};

#endif /* YOKE_H_ */