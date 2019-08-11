#ifndef BRG_SERIAL_H
#define BRG_SERIAL_H

#include "defs.h"

#include <cstring>
#include <string>
#include <type_traits>
#include <ostream>

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

template<>
std::string deserializeBackwards<std::string>(const byte*& buf)
{
    auto size = deserializeBackwards<std::size_t>(buf);
    buf -= size;
    return std::string(buf, size);
}

template<class T>
void serializeForBackward(std::ostream& os, const T& t)
{
    static_assert(std::is_trivial<T>::value, "Can only write trivial types as binary data");
    os.write(reinterpret_cast<const byte*>(&t), sizeof(t));
}

template<>
void serializeForBackward(std::ostream& os, const std::string& str)
{
    os.write(str.c_str(), str.size());
    serializeForBackward(os, str.size());
}

}

#endif
