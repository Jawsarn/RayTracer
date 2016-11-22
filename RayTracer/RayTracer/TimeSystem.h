#pragma once
#include <chrono>

class TimeSystem
{
public:

    static TimeSystem * GetInstance();

    static void Startup();

    void StartClock();

    void Tick();

    void UpdateAccumulatorAndGameTime();

    double GetFrameAlpha();

    bool ShouldUpdateFrame();

    bool IsLagging();

    void ResetTime();

    bool FrameLessThenTimeStep();

    void SleepTillNextUpdate();

    double GetStepLength() { return UpdateStepLen; }
    

private:
    TimeSystem();
    ~TimeSystem();

    static TimeSystem* m_singleton;

    double Frame = 0;
    double LaggTime = 0;
    double Accum = 0;
    double GameTime = 0;
    double UpdateStepLen = 0.017f;
    double MaxFrameTime = 0.25f;

    std::chrono::time_point<std::chrono::high_resolution_clock> CurrentClock;
    std::chrono::time_point<std::chrono::high_resolution_clock> PreviousClock;

};

