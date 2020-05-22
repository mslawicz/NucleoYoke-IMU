#ifndef FILTER_H_
#define FILTER_H_

#include <mbed.h>
#include <vector>

/*
Simple Moving Average filter
*/
class FilterSMA
{
public:
    FilterSMA(size_t filterSize);
    void calculate(float input);
    float getValue(void) const { return filterValue; }
private:
    size_t filterSize;                      // size of the averaging buffer
    std::vector<float> dataBuffer;          // data buffer for filter calculations
    size_t currentElement{0};               // index of the current element
    float filterValue{0.0f};                // current filter value
    float filterFactor{0.0f};               // reciprocal of filter size for calculation purposes
};

/*
Adaptive Exponential Moving Average filter
*/
class FilterAEMA
{
public:
    void calculate(float input);
    float getValue(void) const { return filterValue; }
private:
    const float FilterStrength = 0.03f;
    float filterValue{0.0f};      // current filtered value
    const float DeviationFilterStrength = 0.02f;        // filter strength for average deviation calculations
    float filteredDeviation{0.0f};  // current filtered deviation value 
};

#endif /* FILTER_H_ */