#ifndef HX711_H_
#define HX711_H_

#include <mbed.h>

class HX711
{
public:
    HX711(PinName dataPin, PinName clockPin, EventQueue& eventQueue, uint8_t totalPulses = 25);
    uint32_t getDataRegister(void) const { return dataRegister; }
    float getValue(void) const { return 1.0f * static_cast<int32_t>(dataRegister << 8) / 0x7FFFFF00; }
private:
    void read(void);            // reads data from HX711 chip
    InterruptIn data;           // interrupt and data pin
    DigitalOut clock;           // clock pin
    EventQueue& eventQueue;     // event queue for HX711 data readouts
    uint8_t totalPulses;        // number of pulses to generate <25-27>
    uint32_t dataRegister{0};   // data read from the chip
};

#endif /* HX711_H_ */