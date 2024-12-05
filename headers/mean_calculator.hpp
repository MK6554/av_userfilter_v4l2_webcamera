#ifndef MEANCALCULATOR
#define MEANCALCULATOR
#include <array>
class MeanCalculator
{
    static int const CalculatorSize = 50;

private:
    int currentIndex = 0;
    std::array<double, CalculatorSize> aggregate;

public:
MeanCalculator();
    void append(double value);
    double getMean() const;
};

#endif /* MEANCALCULATOR */
