#ifndef SERVO_H_
#define SERVO_H_

#include <mbed.h>

//min, -90, 0, 90, max [ms]
//DS3218 (180): 0.45, 0.55, 1.5, 2.51, 2.55
//RDS3225 (270): 0.47, 0.87, 1.5, 2.17, 2.46
//RDS3235 (180): 0.48, 0.57, 1.5, 2.45, 2.56

class Servo
{
public:
    Servo(PinName pwmPin, float minPulseWidth, float maxPulseWidth, float startValue, bool reverse = false);
    void test(float pulseWidth) { pwmOut.pulsewidth(pulseWidth); }
    void setValue(float value);
private:
    PwmOut pwmOut;
    float minPulseWidth;    // minimum pulse width [s]
    float maxPulseWidth;    // maximum pulse width [s]
    float startValue;       // start value in the range <0..1>
    bool reverse;           // reverse direction of moving
};

#endif /* SERVO_H_ */