#include "crypt.h"

#include <cstring>
#include <memory>

void brg::encrypt(const char* keyP, brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    auto key = std::unique_ptr<char[]>(::strdup(keyP));
    auto keylen = ::strlen(keyP);

    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        auto d = (unsigned char)data[i];
        d += key[i%keylen];
        out[i] = (brg::byte)d;
        key[i%keylen] += d;
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}

void brg::decrypt(const char* keyP, brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    auto key = std::unique_ptr<char[]>(::strdup(keyP));
    auto keylen = ::strlen(keyP);

    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        auto d = (unsigned char)data[i];
        d -= key[i%keylen];
        out[i] = (brg::byte)d;
        key[i%keylen] += data[i];
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}
