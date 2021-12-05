#include "Filter.h"

FilterSMA::FilterSMA(size_t filterSize) :
    filterSize(filterSize),
    dataBuffer(filterSize, 0.0F)
{
    MBED_ASSERT(filterSize);
    filterFactor = 1.0F / static_cast<float>(filterSize);
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
    filteredDeviation = filteredDeviation * (1.0F - DeviationFilterStrength) + fabs(delta) * DeviationFilterStrength;
    float alpha = FilterStrength * fabs(delta) / filteredDeviation;
    if(alpha > 1.0F)
    {
        alpha = 1.0F;
    }
    filterValue += delta * alpha;
}

void FilterEMA::calculate(float input)
{
    filterValue += filterFactor * (input - filterValue);
}

FilterMM::FilterMM(size_t size) :
    size(size)
{
    unsortedBuffer.assign(size, 0.0F);
    sortedBuffer.assign(size, 0.0F);
}

void FilterMM::calculate(float newInputValue)
{
    // delete the oldest element from the sorted buffer
    float oldestValue = unsortedBuffer[headIndex];
    for (auto element = sortedBuffer.begin(); element != sortedBuffer.end(); element++)
    {
        if (*element == oldestValue)
        {
            sortedBuffer.erase(element);
            break;
        }
    }

    // emplace the new element
    size_t step = size / 2;
    size_t searchIndex = size / 2;
    while ((searchIndex > 0) && (searchIndex < sortedBuffer.size()))
    {
        if (step > 1)
        {
            step /= 2;
        }

        if (newInputValue > sortedBuffer[searchIndex])
        {
            searchIndex += step;
        }
        else
        {
            if (newInputValue > sortedBuffer[searchIndex - 1])
            {
                break;
            }
            searchIndex -= step;
        }
    }
    if (searchIndex >= sortedBuffer.size())
    {
        sortedBuffer.push_back(newInputValue);
    }
    else
    {
        sortedBuffer.insert(sortedBuffer.begin() + searchIndex, newInputValue);     //NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
    }
    unsortedBuffer[headIndex] = newInputValue;
    headIndex = (headIndex + 1) % size;
}
