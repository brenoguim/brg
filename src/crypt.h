#ifndef BRG_CRYPT_H
#define BRG_CRYPT_H

#include "defs.h"
#include <functional>

namespace brg
{

void encrypt(span<const byte> data, const std::function<void(span<const byte>)>& writeOut);
void decrypt(span<const byte> data, const std::function<void(span<const byte>)>& writeOut);

}

#endif
