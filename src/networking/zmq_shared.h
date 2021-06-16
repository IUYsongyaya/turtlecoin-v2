// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_SHARED_H
#define TURTLECOIN_NETWORKING_ZMQ_SHARED_H

#include <atomic>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <zmq.hpp>

namespace Networking
{
    class zmq_connection_monitor : public zmq::monitor_t
    {
      public:
        ~zmq_connection_monitor()
        {
            m_running = false;

            if (m_poller.joinable())
            {
                m_poller.join();
            }
        }

        void on_event_connected(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_connected_peers.insert(addr);

            m_delayed_peers.erase(addr);

            m_retried_peers.erase(addr);
        }

        void on_event_connect_delayed(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_delayed_peers.insert(addr);

            m_retried_peers.erase(addr);

            m_connected_peers.erase(addr);
        }

        void on_event_connect_retried(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_retried_peers.insert(addr);

            m_delayed_peers.erase(addr);

            m_connected_peers.erase(addr);
        }

        void on_event_listening(const zmq_event_t &event, const char *addr) override
        {
            m_listening = true;
        }

        void on_event_accepted(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_connected_peers.insert(addr);
        }

        void on_event_closed(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_connected_peers.erase(addr);
        }


        void on_event_disconnected(const zmq_event_t &event, const char *addr) override
        {
            std::scoped_lock lock(m_mutex);

            m_connected_peers.erase(addr);
        }

        std::set<std::string> connected() const
        {
            std::scoped_lock lock(m_mutex);

            return m_connected_peers;
        }

        std::set<std::string> delayed() const
        {
            std::scoped_lock lock(m_mutex);

            return m_delayed_peers;
        }

        void join()
        {
            m_running = false;

            if (m_poller.joinable())
            {
                m_poller.join();
            }
        }

        bool listening()
        {
            return m_listening;
        }

        std::set<std::string> retried() const
        {
            std::scoped_lock lock(m_mutex);

            return m_retried_peers;
        }

        void start()
        {
            m_running = true;

            m_poller = std::thread(&zmq_connection_monitor::listener, this);
        }

      private:
        void listener()
        {
            while (m_running)
            {
                this->check_event(100);
            }
        }

        mutable std::mutex m_mutex;

        std::atomic<bool> m_running = false, m_listening = false;

        std::set<std::string> m_connected_peers, m_delayed_peers, m_retried_peers;

        std::thread m_poller;
    };
} // namespace Networking

#endif
