// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_THREAD_HELPER_H
#define TURTLECOIN_THREAD_HELPER_H

#include <chrono>
#include <condition_variable>
#include <config.h>
#include <mutex>
#include <thread>

/**
 * Pauses execution of the running thread until the timeout elapses OR
 * a signal is received via the conditional variable
 *
 * Returns (TRUE) if we received the conditional variable signal
 *
 * @param cv
 * @param milliseconds
 * @return
 */
static inline bool
    thread_sleep(std::condition_variable &cv, size_t milliseconds = Configuration::THREAD_POLLING_INTERVAL)
{
    static std::mutex mutex;

    std::unique_lock<std::mutex> lock(mutex);

    const auto timeout = cv.wait_for(lock, std::chrono::milliseconds(milliseconds));

    return timeout != std::cv_status::timeout;
}

#endif
