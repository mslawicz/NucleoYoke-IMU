#include "HX711.h"

/*
create HX711 object and set interrupt
use unique dataPin bus!
*/
HX711::HX711(PinName dataPin, PinName clockPin, EventQueue& eventQueue, uint8_t totalPulses) :
    data(dataPin, PullUp),
    clock(clockPin, 0),
    eventQueue(eventQueue),
    totalPulses(totalPulses)
{
    MBED_ASSERT((totalPulses >= 25) && (totalPulses <= 27));
}

/*
read data from the HX711 chip
it should be called in a separate low priority thread
*/
void HX711::read(void)
{
    uint32_t dataBuffer = 0;
    // generate clock pulses and read data bits
    for(uint8_t pulse = 0; pulse < totalPulses; pulse++)
    {
        // the pulse mustn't be interrupted (maximum length 50 us)
        CriticalSectionLock lock;
        clock = 1;
        if(pulse < 24)      // read 24 bits of data
        {
            dataBuffer = (dataBuffer << 1) | data.read();
        }
        clock = 0;
    }

    dataRegister = dataBuffer;
}

/*
check if new data is ready to read and request readout in the dedicated thread
*/
void HX711::readRequest(void)
{
    if(data.read() == 0)
    {
        eventQueue.call(callback(this, &HX711::read));
    }
}
