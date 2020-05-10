#include "Encoder.h"

// XXX object test
Encoder testEncoder(PG_2, PG_3);
static DigitalOut testLED(LED3);

Encoder::Encoder(PinName dataPin, PinName clkPin) :
    data(dataPin, PullUp),
    clk(clkPin, PullUp)
{
    enableInterrupts();
}

/*
callback called on falling clock signal
*/
void Encoder::onClockChangeCb(void)
{
    // disable next interrupts until it's intentionally enabled again
    clk.fall(nullptr);
    clk.rise(nullptr);
    // enable this interrupt after timeout
    interruptEnableTimeout.attach(callback(this, &Encoder::enableInterrupts), 0.1f);

    // execute user on data read callback on falling edge only
    if(clk == 0)
    {
        // XXX LED test
        testLED = !testLED;
    }
}

void Encoder::enableInterrupts(void)
{
    clk.fall(callback(this, &Encoder::onClockChangeCb));
    clk.rise(callback(this, &Encoder::onClockChangeCb));
}