#include "timer.h"

Timer::Timer(int historySize)
    : maxHistorySize(historySize)
{
}

void Timer::start(const std::string &name)
{
    if (timers.find(name) == timers.end())
    {
        timers.emplace(name, TimerData(maxHistorySize));
    }

    TimerData &data = timers.at(name);

    if (data.running)
    {
        return;
    }

    data.startTime = std::chrono::high_resolution_clock::now();
    data.running = true;
}

long long Timer::stop(const std::string &name)
{
    auto it = timers.find(name);
    if (it == timers.end() || !it->second.running)
    {
        return -1;
    }

    TimerData &data = it->second;
    auto endTime = std::chrono::high_resolution_clock::now();
    data.running = false;

    long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
                             endTime - data.startTime)
                             .count();

    data.durationHistory[data.currentIndex] = duration;
    data.currentIndex = (data.currentIndex + 1) % maxHistorySize;

    return duration;
}

long long Timer::getLastDuration(const std::string &name) const
{
    auto it = timers.find(name);
    if (it == timers.end())
    {
        return -1;
    }

    const TimerData &data = it->second;
    size_t lastIndex = (data.currentIndex == 0) ? maxHistorySize - 1 : data.currentIndex - 1;
    return data.durationHistory[lastIndex];
}

const std::vector<long long> &Timer::getDurationHistory(const std::string &name) const
{
    static const std::vector<long long> emptyVector(maxHistorySize, 0);
    auto it = timers.find(name);
    return (it != timers.end()) ? it->second.durationHistory : emptyVector;
}

double Timer::getAverageDuration(const std::string &name) const
{
    auto it = timers.find(name);
    if (it == timers.end())
    {
        return 0.0;
    }

    const std::vector<long long> &history = it->second.durationHistory;
    double sum = 0.0;
    size_t validEntries = 0;

    for (const auto &duration : history)
    {
        if (duration > 0)
        {
            sum += duration;
            validEntries++;
        }
    }

    return validEntries > 0 ? sum / validEntries : 0.0;
}
