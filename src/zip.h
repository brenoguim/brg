#ifndef BRG_ZIP_H
#define BRG_ZIP_H

#include "defs.h"
#include <functional>

namespace brg
{

void zip(span<const byte> data, const std::function<void(span<const byte>)>& writeOut);
void unzip(span<const byte> data, const std::function<void(span<const byte>)>& writeOut);

}

#endif
