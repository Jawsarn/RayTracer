#include "TimeSystem.h"
#include <stdexcept>
#include <iostream>
#include <thread>



TimeSystem* TimeSystem::m_singleton = nullptr;

TimeSystem* TimeSystem::GetInstance()
{
    if (m_singleton == nullptr)
        throw std::runtime_error("Non-initialized time system received");

    return m_singleton;
}

void TimeSystem::Startup()
{
    if (m_singleton != nullptr)
        throw std::runtime_error("Startup called multiple times");

    m_singleton = new TimeSystem();
}

void TimeSystem::StartClock()
{
    PreviousClock = std::chrono::high_resolution_clock::now();
}

void TimeSystem::Tick()
{
    CurrentClock = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = (CurrentClock - PreviousClock);
    Frame = duration.count() + LaggTime;
    LaggTime = 0.0f;

    // We simulate maximum 250 milliseconds each frame
    // If we would let it be alone we would get mayor stops instead of lesser ones that will slowly catch up
    if (Frame > MaxFrameTime)
    {
        LaggTime = Frame - MaxFrameTime;
        Frame = MaxFrameTime;
        std::cout << "Frame took more then " << MaxFrameTime << " Seconds" << std::endl;
    }

    // Update the previous position with frametime so we can catch up if we slow down
    PreviousClock = CurrentClock;

    // Update Accumulator (how much we will work this frame)
    Accum += Frame;
}

void TimeSystem::UpdateAccumulatorAndGameTime()
{
    // Remove time from accumulator
    // Accumulator -= UpdateTimeStepLength;
    Accum -= UpdateStepLen;

    // Add time to start
    GameTime += UpdateStepLen;
}

double TimeSystem::GetFrameAlpha() { return Accum / UpdateStepLen; }

bool TimeSystem::ShouldUpdateFrame() { return Accum >= UpdateStepLen; }

bool TimeSystem::IsLagging()
{
    // If we have any lagg time or the frame took more then one update, we are lagging
    // TODOXX if we're moving at exaclty 60 fps, this might not work too well
    // So we keep it at 3 times for now.. that like 20 fps
    if (LaggTime > 0.0f || Frame > UpdateStepLen * 3.0f)
    {
        return false;
    }
    return false;
}

void TimeSystem::ResetTime()
{
    CurrentClock = std::chrono::high_resolution_clock::now();

    // Update the previous position with frametime so we can catch up if we slow down
    PreviousClock = CurrentClock;
}

bool TimeSystem::FrameLessThenTimeStep() { return Frame < UpdateStepLen; }

void TimeSystem::SleepTillNextUpdate()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uint32_t>((UpdateStepLen - Frame) * 1000.0f)));
}

TimeSystem::TimeSystem() {}

TimeSystem::~TimeSystem() {}