// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_THREAD_SAFE_PRIORITY_QUEUE_H
#define TURTLECOIN_THREAD_SAFE_PRIORITY_QUEUE_H

#include <mutex>
#include <queue>
#include <vector>

template<typename T, typename Comparison = std::less<T>> class ThreadSafePriorityQueue
{
  public:
    ThreadSafePriorityQueue() {}

    /**
     * Returns whether the queue is empty
     *
     * @return
     */
    bool empty() const
    {
        std::scoped_lock lock(m_mutex);

        return m_priority_queue.empty();
    }

    /**
     * Removes the top element in queue and returns it to the caller
     *
     * @return
     */
    T pop()
    {
        std::scoped_lock lock(m_mutex);

        auto item = m_priority_queue.top();

        m_priority_queue.pop();

        return item;
    }

    /**
     * Adds the element to the queue
     *
     * @param item
     */
    void push(const T &item)
    {
        std::scoped_lock lock(m_mutex);

        m_priority_queue.push(item);
    }

    /**
     * Adds the vector of elements to the queue
     * in the order in which they were received
     *
     * @param items
     */
    void push(const std::vector<T> &items)
    {
        std::scoped_lock lock(m_mutex);

        for (const auto &item : items)
        {
            m_priority_queue.push(item);
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

        return m_priority_queue.size();
    }

    /**
     * Returns the top element in the queue without removing it
     *
     * @return
     */
    T top() const
    {
        std::scoped_lock lock(m_mutex);

        return m_priority_queue.top();
    }

  private:
    std::priority_queue<T, std::vector<T>, Comparison> m_priority_queue;

    mutable std::mutex m_mutex;
};

#endif
