#include "crypt.h"

#include <memory>

void brg::encrypt(brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    brg::byte key = 33;
    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        out[i] = data[i] + key;
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}

void brg::decrypt(brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    brg::byte key = 33;
    std::unique_ptr<brg::byte[]> out(new brg::byte[data.size()]);
    for (std::size_t i = 0; i < data.size(); ++i) {
        out[i] = data[i] - key;
    }
    writeOut(brg::span<const brg::byte>(out.get(), data.size()));
}
