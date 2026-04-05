#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <cstddef>

template <typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size) : m_data(size), m_size(size), m_head(0), m_count(0) {}

    void push(const T& item) {
        m_data[m_head] = item;
        m_head = (m_head + 1) % m_size;
        if (m_count < m_size) m_count++;
    }

    T get_last() const {
        if (m_count == 0) return T{};
        return m_data[(m_head + m_size - 1) % m_size];
    }

    T get_past(size_t steps_ago) const {
        if (steps_ago >= m_count) return T{};
        size_t index = (m_head + m_size - 1 - steps_ago) % m_size;
        return m_data[index];
    }

    size_t count() const { return m_count; }
    size_t size() const { return m_size; }

private:
    std::vector<T> m_data;
    size_t m_size;
    size_t m_head;
    size_t m_count;
};

#endif
