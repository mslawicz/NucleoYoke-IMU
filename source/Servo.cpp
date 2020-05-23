#include "Servo.h"
#include "Scale.h"

/** create servo object
 *
 * @param pwmPin PwmOut pin to connect to
 * @param minPulseWidth float minimum input value in [ms]
 * @param maxPulseWidth float maximum input value in [ms]
 * @param startValue float start value in the range 0..1
 * @param reverse bool reverses the direction of servo movement
 */
Servo::Servo(PinName pwmPin, float minPulseWidth, float maxPulseWidth, float startValue, bool reverse) :
    pwmOut(pwmPin),
    minPulseWidth(minPulseWidth),
    maxPulseWidth(maxPulseWidth),
    startValue(startValue),
    reverse(reverse)
{
    pwmOut.period(0.01f);
    setValue(startValue);
}

/*
 * set servo value in the range <0..1>
 */
void Servo::setValue(float value)
{
    pwmOut.pulsewidth(scale<float, float>(0.0f, 1.0f, reverse ? (1.0f-value) : value, minPulseWidth, maxPulseWidth));
}