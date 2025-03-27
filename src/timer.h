/*! \file timer.h
 * \brief Calculate time consumption using RAII
 */

#pragma once

#include <chrono>
#include <iostream>

class Timer {
    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

public:
    /**
     * \brief Creates a new Timer object.
     *
     * Initialize creation time which is used when a created Timer object
     *
     */
    Timer();
    /**
     * \brief Destroy an object and print the time
     *
     * When an object is exited from a scope it prints a lifetime of the object
     *
     */
    ~Timer();

private:
    //! \brief time point initializes at the start of a creating Timer object
    const std::chrono::time_point<std::chrono::system_clock> startTime;
};
