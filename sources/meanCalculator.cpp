#include "meanCalculator.hpp"
#include <cstdio>
void MeanCalculator::append(double value)
{
    aggregate[currentIndex % CalculatorSize] = value;
    currentIndex++;
}
double MeanCalculator::getMean() const
{
    double sum = 0;
    for (auto const &v : aggregate)
    {
        sum += v;
    }
    auto div = currentIndex >= CalculatorSize ? CalculatorSize : currentIndex;
    return sum / CalculatorSize;
}

MeanCalculator::MeanCalculator()
{
    for (size_t i = 0; i < CalculatorSize; i++)
    {
        aggregate[i] = 0;
    }
}