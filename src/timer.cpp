/*! \file timer.cpp
 * \brief Timer class implementation.
 */
#include "timer.h"

Timer::Timer() : startTime(std::chrono::system_clock::now()) {}

Timer::~Timer() {
    // Calculate duration
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - startTime);
    // Extract seconds and milliseconds
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    const auto milliseconds = duration - seconds;
    // Print the results
    std::cout << "Process time: " << seconds.count() << "s " << milliseconds.count() << " ms\n";
}