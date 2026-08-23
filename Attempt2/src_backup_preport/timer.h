#pragma once
#include <chrono>

class Timer
{
public:
    Timer() { reset(); }

    void reset()
    {
        start = std::chrono::steady_clock::now();
    }

    float delta()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = now - start;
        start = now;
        return diff.count(); // seconds
    }

private:
    std::chrono::steady_clock::time_point start;
};