// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_THREAD_SAFE_SET_H
#define TURTLECOIN_THREAD_SAFE_SET_H

#include <mutex>
#include <set>
#include <thread>

template<typename T> class ThreadSafeSet
{
  public:
    ThreadSafeSet() {}

    /**
     * returns an iterator to the beginning
     *
     * @return
     */
    auto begin() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.begin();
    }

    /**
     * Removes all elements from the container
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);

        m_container.clear();
    }

    /**
     * checks if the container contains element with specific key
     *
     * @param key
     * @return
     */
    bool contains(const T &key)
    {
        std::scoped_lock lock(m_mutex);

        return m_container.count(key) != 0;
    }

    /**
     * Returns whether the container is empty
     *
     * @return
     */
    bool empty() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.empty();
    }

    /**
     * returns an iterator to the end
     *
     * @return
     */
    auto end() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.end();
    }

    /**
     * erases elements
     *
     * @param key
     */
    void erase(const T &key)
    {
        std::scoped_lock lock(m_mutex);

        m_container.erase(key);
    }

    /**
     * finds element with specific key
     *
     * @param key
     * @return
     */
    auto find(const T &key)
    {
        std::scoped_lock lock(m_mutex);

        return m_container.find(key);
    }

    /**
     * inserts elements
     *
     * @param key
     */
    void insert(const T &key)
    {
        std::scoped_lock lock(m_mutex);

        m_container.insert(key);
    }

    /**
     * Returns the maximum possible number of elements for the container
     *
     * @return
     */
    size_t max_size() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.max_size();
    }

    /**
     * returns a reverse iterator to the beginning
     *
     * @return
     */
    auto rbegin() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.rbegin();
    }

    /**
     * returns a reverse iterator to the end
     *
     * @return
     */
    auto rend() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.rend();
    }

    /**
     * Returns the size of the container
     *
     * @return
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.size();
    }

  private:
    std::set<T> m_container;

    mutable std::mutex m_mutex;
};

#endif
