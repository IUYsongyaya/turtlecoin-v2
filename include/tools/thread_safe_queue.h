// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_THREAD_SAFE_QUEUE_H
#define TURTLECOIN_THREAD_SAFE_QUEUE_H

#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template<typename T> class ThreadSafeQueue
{
  public:
    ThreadSafeQueue() {}

    /**
     * Returns the last element in the queue
     *
     * @return
     */
    T back() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.back();
    }

    /**
     * Removes all elements from the queue
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);

        m_container = std::queue<T>();
    }

    /**
     * Returns whether the queue is empty
     *
     * @return
     */
    bool empty() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.empty();
    }

    /**
     * Returns the first element in the queue without removing it
     *
     * @return
     */
    T front() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.front();
    }

    /**
     * Removes the first element in queue and returns it to the caller
     *
     * @return
     */
    T pop()
    {
        std::scoped_lock lock(m_mutex);

        auto item = m_container.front();

        m_container.pop();

        return item;
    }

    /**
     * Adds the element to the end of the queue
     *
     * @param item
     */
    void push(const T &item)
    {
        std::scoped_lock lock(m_mutex);

        m_container.push(item);
    }

    /**
     * Adds the vector of elements to the end of the queue
     * in the order in which they were received
     *
     * @param items
     */
    void push(const std::vector<T> &items)
    {
        std::scoped_lock lock(m_mutex);

        for (const auto &item : items)
        {
            m_container.push(item);
        }
    }

    /**
     * Returns the size of the queue
     *
     * @return
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.size();
    }

    /**
     * Removes the first element of the queue
     */
    void skip()
    {
        std::scoped_lock lock(m_mutex);

        if (m_container.empty())
        {
            return;
        }

        m_container.pop();
    }

  private:
    std::queue<T> m_container;

    mutable std::mutex m_mutex;
};

#endif
