#ifndef BRG_DEFS_H
#define BRG_DEFS_H

#include <cstddef>

namespace brg
{
using byte = char;

template <class T>
class span
{
  public:
    span(T* begin, std::size_t size)
        : m_begin(begin)
        , m_end(m_begin + size)
    {}

    span() = default;
    span(const span&) = default;
    span& operator=(const span&) = default;
    span(span&&) = default;
    span& operator=(span&&) = default;

    T* begin() const { return m_begin; }
    T* end() const { return m_end; }
    std::size_t size() const { return m_end - m_begin; }
    bool empty() const { return m_begin == m_end; }
    T operator[](std::size_t i) const { return m_begin[i]; }

    explicit operator bool() const { return !empty(); }

  private:
    T* m_begin {nullptr};
    T* m_end {nullptr};
};

}

#endif
