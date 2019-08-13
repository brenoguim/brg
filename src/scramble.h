#ifndef BRG_SCRAMBLE_H
#define BRG_SCRAMBLE_H

#include "defs.h"
#include <functional>

namespace brg
{

void scramble(const char* key, span<const byte> data, const std::function<void(span<const byte>)>& writeOut);
void descramble(const char* key, span<const byte> data, const std::function<void(span<const byte>)>& writeOut);

}

#endif
