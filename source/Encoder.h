#ifndef ENCODER_H_
#define ENCODER_H_

#include <mbed.h>

class Encoder
{
public:
    Encoder(PinName dataPin, PinName clkPin);
private:
    void onFallingClockCb(void);    // on falling clock callback
    void onDataChangeCb(void);      // on data input change callback 
    InterruptIn data;
    InterruptIn clk;
};

#endif /* ENCODER_H_ */