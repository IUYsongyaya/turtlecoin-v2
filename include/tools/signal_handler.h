// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_SIGNAL_HANDLER_H
#define TURTLECOIN_SIGNAL_HANDLER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <utility>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <signal.h>
#endif

namespace ControlSignal
{
    static std::function<void(void)> m_handler;

    static std::mutex m_mutex;

    static inline void handle_signal()
    {
        if (m_handler)
        {
            std::scoped_lock lock(m_mutex);

            m_handler();
        }
    }

#ifdef WIN32
    static inline BOOL WINAPI handler(DWORD type)
    {
        if (CTRL_C_EVENT == type || CTRL_BREAK_EVENT == type)
        {
            handle_signal();

            return true;
        }

        return false;
    }
#else
    static inline void handler(int type)
    {
        handle_signal();
    }
#endif

    static inline bool register_handler(std::function<void(void)> callback)
    {
#ifdef WIN32
        auto result = TRUE == ::SetConsoleCtrlHandler(&handler, TRUE);

        if (result)
        {
            m_handler = std::move(callback);
        }

        return result;
#else
        signal(SIGINT, handler);

        signal(SIGTERM, handler);

        m_handler = std::move(callback);

        return true;
#endif
    }
} // namespace ControlSignal

#endif
