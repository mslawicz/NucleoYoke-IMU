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
    explicit FilterSMA(size_t filterSize);
    void calculate(float input);
    float getValue() const { return filterValue; }
private:
    size_t filterSize;                      // size of the averaging buffer
    std::vector<float> dataBuffer;          // data buffer for filter calculations
    size_t currentElement{0};               // index of the current element
    float filterValue{0.0F};                // current filter value
    float filterFactor{0.0F};               // reciprocal of filter size for calculation purposes
};

/*
Adaptive Exponential Moving Average filter
*/
class FilterAEMA
{
public:
    void calculate(float input);
    float getValue() const { return filterValue; }
private:
    const float FilterStrength = 0.03F;
    float filterValue{0.0F};      // current filtered value
    const float DeviationFilterStrength = 0.02F;        // filter strength for average deviation calculations
    float filteredDeviation{0.0F};  // current filtered deviation value 
};

/*
Regular Exponential Moving Average filter
*/
class FilterEMA
{
public:
    explicit FilterEMA(float filterFactor) : filterFactor(filterFactor) {}
    void calculate(float input);
    float getValue() const { return filterValue; }
    void setFactor(float factor) { filterFactor = factor; }
private:
    float filterFactor;
    float filterValue{0.0F};      // current filtered value
};

class FilterMM
{
public:
    explicit FilterMM(size_t size);
    void displayBuffer();
    float getValue() { return sortedBuffer[size / 2]; }
    void calculate(float newInputValue);
private:
    size_t size;
    size_t headIndex{ 0 };
    std::vector<float> unsortedBuffer;
    std::vector<float> sortedBuffer;
};

#endif /* FILTER_H_ */