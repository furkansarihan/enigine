#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

class Timer
{
public:
    Timer(int historySize = 10);

    void start(const std::string &name);
    long long stop(const std::string &name);

    long long getLastDuration(const std::string &name) const;
    const std::vector<long long> &getDurationHistory(const std::string &name) const;
    double getAverageDuration(const std::string &name) const;

    struct TimerData
    {
        bool running = false;
        int currentIndex = 0;
        std::vector<long long> durationHistory;
        std::chrono::high_resolution_clock::time_point startTime;

        TimerData(int historySize)
            : durationHistory(historySize, 0)
        {
        }
    };

    int maxHistorySize;
    std::unordered_map<std::string, TimerData> timers;
};

#endif // TIMER_H
