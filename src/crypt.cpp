#include "crypt.h"

#include <memory>

void brg::encrypt(unsigned char key, brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        auto d = (unsigned char)data[i];
        d += key;
        out[i] = (brg::byte)d;
        key += d;
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}

void brg::decrypt(unsigned char key, brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        auto d = (unsigned char)data[i];
        d -= key;
        out[i] = (brg::byte)d;
        key += data[i];
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}
