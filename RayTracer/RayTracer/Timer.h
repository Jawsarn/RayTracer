#pragma once
#include <chrono>

class Timer
{
public:
    Timer();
    ~Timer();

    void StartTimer();
    void StopTimer();
    double GetTime();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> StopTime;

};

