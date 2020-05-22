#include "Filter.h"

FilterSMA::FilterSMA(size_t filterSize) :
    filterSize(filterSize),
    dataBuffer(filterSize, 0.0f)
{
    MBED_ASSERT(filterSize);
    filterFactor = 1.0f / filterSize;
}

/*
calculate filter value for a new input value
*/
void FilterSMA::calculate(float input)
{
    filterValue += filterFactor * (input - dataBuffer[currentElement]);
    dataBuffer[currentElement] = input;
    currentElement = (currentElement + 1) % filterSize;
}

void FilterAEMA::calculate(float input)
{
    float delta = input - filterValue;
    filteredDeviation = filteredDeviation * (1.0f - DeviationFilterStrength) + fabs(delta) * DeviationFilterStrength;
    float alpha = FilterStrength * fabs(delta) / filteredDeviation;
    if(alpha > 1.0f)
    {
        alpha = 1.0f;
    }
    filterValue += delta * alpha;
}