#ifndef CPUSTATS
#define CPUSTATS
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "meanCalculator.hpp"
class ProcessUsage
{
private:
    int _pid;
    MeanCalculator calc;

    long measureTotalTime();
    long measureProcessTime();

    long lastTotalTime;
    long lastProcessTime;

public:
    ProcessUsage(int pid) : _pid(pid) {}
    double measure();
    double getMeanUsage() const;
};

#endif /* CPUSTATS */
