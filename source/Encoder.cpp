#include "Encoder.h"

// XXX object test
Encoder testEncoder(PG_2, PG_3);
static DigitalOut testLED(LED3);

Encoder::Encoder(PinName dataPin, PinName clkPin) :
    data(dataPin, PullUp),
    clk(clkPin, PullUp)
{
    clk.fall(callback(this, &Encoder::onFallingClockCb));
}

/*
callback called on falling clock signal
*/
void Encoder::onFallingClockCb(void)
{
    // disable next interrupts until it's intentionally enabled again
    clk.fall(nullptr);
    // enable on data change callback
    data.rise(callback(this, &Encoder::onDataChangeCb));
    data.fall(callback(this, &Encoder::onDataChangeCb));

    // execute user on data read callback
    // XXX LED test
    testLED = !testLED;
}

/*
callback called on data input change
*/
void Encoder::onDataChangeCb(void)
{
    // disable next interrupts until it's intentionally enabled again
    data.rise(nullptr);
    data.fall(nullptr);
    // enable on falling clock interrupts
    clk.fall(callback(this, &Encoder::onFallingClockCb));
}