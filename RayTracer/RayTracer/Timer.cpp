#include "Timer.h"



Timer::Timer()
{
}


Timer::~Timer()
{
}

void Timer::StartTimer()
{
    StartTime = std::chrono::high_resolution_clock::now();
}

void Timer::StopTimer()
{
    StopTime = std::chrono::high_resolution_clock::now();
}

double Timer::GetTime()
{
    std::chrono::duration<double> duration = (StopTime - StartTime);
    return duration.count();
}
