#ifndef ENCODER_H_
#define ENCODER_H_

#include <mbed.h>

class Encoder
{
public:
    Encoder(PinName dataPin, PinName clkPin);
private:
    PinName dataPin;
    PinName clkPin;
};

#endif /* ENCODER_H_ */