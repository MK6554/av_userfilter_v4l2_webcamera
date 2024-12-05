#include "cpuStats.hpp"
#ifdef PLATFORM_UNIX
long ProcessUsage::measureTotalTime()
{
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);

    std::istringstream ss(line);
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    lastTotalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    return lastTotalTime;
}

long ProcessUsage::measureProcessTime()
{
    std::ifstream file("/proc/" + std::to_string(_pid) + "/stat");
    std::string line;
    std::getline(file, line);

    std::istringstream ss(line);
    std::string token;
    long utime, stime;
    for (int i = 0; i < 13; ++i)
        ss >> token; // Skip first 13 fields
    ss >> utime >> stime;

    lastProcessTime = utime + stime;
    return lastProcessTime;
}

double ProcessUsage::measure()
{
    auto prevTotal = lastTotalTime;
    auto prevProcess = lastProcessTime;

    auto newTotal = measureTotalTime();
    auto newProcess = measureProcessTime();

    auto diffTotal = newTotal - prevTotal;
    auto diffProcess = newProcess - prevProcess;

    auto usage = (double)diffProcess / diffTotal * 100.0;

    calc.append(usage);
    return usage;
}

double ProcessUsage::getMeanUsage()const{
    return calc.getMean();
}
#else
long ProcessUsage::measureTotalTime(){return 1;}
long ProcessUsage::measureProcessTime(){return 1;}
double ProcessUsage::measure(){return 100;}
double ProcessUsage::getMeanUsage()const{return 100;}
#endif
