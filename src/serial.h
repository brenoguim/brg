#ifndef BRG_SERIAL_H
#define BRG_SERIAL_H

#include "defs.h"

#include <cstring>
#include <string>
#include <type_traits>

namespace brg
{

template<class T>
T deserializeBackwards(const byte*& buf)
{
    static_assert(std::is_trivial<T>::value, "Can only read trivial things");
    buf -= sizeof(T);
    T t;
    std::memcpy(&t, buf, sizeof(T));
    return t;
}

template<class T>
T deserializeBackwards(const byte*& buf, std::size_t size);

template<>
std::string deserializeBackwards<std::string>(const byte*& buf, std::size_t size)
{
    buf -= size;
    return std::string(buf, size);
}

}

#endif
