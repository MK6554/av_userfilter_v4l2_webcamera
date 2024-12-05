#ifndef CPUSTATS
#define CPUSTATS
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "mean_calculator.hpp"
class CpuStats
{
private:
    int _pid;
    MeanCalculator calc;

    long measureTotalTime();
    long measureProcessTime();

    long lastTotalTime;
    long lastProcessTime;

public:
    CpuStats();
    double measure();
    double getMeanUsage() const;
};

#endif /* CPUSTATS */
