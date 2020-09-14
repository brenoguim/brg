#pragma once

#include <algorithm>
#include <new>

namespace brg
{

template<class Prefix, class ArrayElement>
class PrefixedArray
{
    PrefixedArray() = default;

    template<class... Args>
    PrefixedArray(Args... args) : m_prefix(std::forward<Args>(args)...) {}

    static std::size_t numBytes(std::size_t arraySize)
    {
        return sizeof(PrefixedArray) + sizeof(ArrayElement)*(arraySize - 1);
    }

  public:
    PrefixedArray(const PrefixedArray&) = delete;
    PrefixedArray(PrefixedArray&&) = delete;
    PrefixedArray& operator=(const PrefixedArray&) = delete;
    PrefixedArray& operator=(PrefixedArray&&) = delete;

    Prefix& prefix() { return m_prefix; }
    const Prefix& prefix() const { return m_prefix; }

    template<class... PrefixArgs>
    static PrefixedArray* make(std::size_t arraySize, PrefixArgs&&... prefixArgs)
    {
        if (arraySize == 0) arraySize = 1;

        void* memBlob = operator new(numBytes(arraySize), std::align_val_t(alignof(PrefixedArray)));

        auto prefixedArray = new (memBlob) PrefixedArray(std::forward<PrefixArgs>(prefixArgs)...);

        for (std::size_t i = 0; i < arraySize; ++i)
        {
            new (&prefixedArray->m_array + i) ArrayElement; 
        }
        return prefixedArray;
    }

    static void destroy(PrefixedArray* pa, std::size_t arraySize)
    {
        if (!pa) return;
        std::destroy_n(&pa->m_array, arraySize);
        pa->~PrefixedArray();
        operator delete(pa, numBytes(arraySize), std::align_val_t(alignof(PrefixedArray)));
    }

  private:
    Prefix m_prefix;
    union { ArrayElement m_array; };
};

template<class PrefixedArray>
void destroy(PrefixedArray* pa, std::size_t sz) { PrefixedArray::destroy(pa, sz); }

} // namespace brg
