// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_THREAD_SAFE_MAP_H
#define TURTLECOIN_THREAD_SAFE_MAP_H

#include <map>
#include <mutex>
#include <thread>

template<typename L, typename R> class ThreadSafeMap
{
  public:
    ThreadSafeMap() {}

    /**
     * Returns the element at the specified key in the container
     *
     * @param key
     * @return
     */
    R at(L key) const
    {
        std::scoped_lock lock(m_mutex);

        return m_container.at(key);
    }

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
    bool contains(const L &key)
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
    void erase(const L &key)
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
    auto find(const L &key)
    {
        std::scoped_lock lock(m_mutex);

        return m_container.find(key);
    }

    /**
     * inserts elements
     *
     * @param key
     * @param value
     */
    void insert(const L &key, const R &value)
    {
        std::scoped_lock lock(m_mutex);

        m_container.insert({key, value});
    }

    /**
     * inserts elements
     *
     * @param kv
     */
    void insert(const std::tuple<L, R> &kv)
    {
        std::scoped_lock lock(m_mutex);

        m_container.insert(kv);
    }

    /**
     * inserts an element or assigns to the current element if the key already exists
     *
     * @param key
     * @param value
     */
    void insert_or_assign(const L &key, const R &value)
    {
        std::scoped_lock lock(m_mutex);

        m_container.insert_or_assign(key, value);
    }

    /**
     * inserts an element or assigns to the current element if the key already exists
     *
     * @param kv
     */
    void insert_or_assign(const std::tuple<L, R> &kv)
    {
        std::scoped_lock lock(m_mutex);

        const auto [key, value] = kv;

        m_container.insert_or_assign(key, value);
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
    std::map<L, R> m_container;

    mutable std::mutex m_mutex;
};

#endif
