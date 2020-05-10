#ifndef ENCODER_H_
#define ENCODER_H_

#include <mbed.h>

class Encoder
{
public:
    Encoder(PinName dataPin, PinName clkPin);
private:
    void enableInterrupts(void);
    void onClockChangeCb(void);    // on clock change callback
    DigitalIn data;
    InterruptIn clk;
    Timeout interruptEnableTimeout;
};

#endif /* ENCODER_H_ */