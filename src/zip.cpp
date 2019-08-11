#include "zip.h"

#include <stdexcept>
#include <string>
#include <zlib.h>
#include <iostream>
#include <vector>

void brg::zip(brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    auto* buf = data.begin();
    auto sz = data.size();

    z_stream gz;
    gz.zalloc = Z_NULL;
    gz.zfree = Z_NULL;
    gz.opaque = Z_NULL;

    gz.next_in = (unsigned char*)buf;
    gz.avail_in = sz;
    deflateInit2(&gz, Z_DEFAULT_COMPRESSION,
                      Z_DEFLATED,
                      MAX_WBITS+16,
                      8,
                      Z_DEFAULT_STRATEGY);

    std::vector<brg::byte> vec;
    char bytebuf[16*1024];
    int r;
    int flush = Z_STREAM_END;
    do
    {
        gz.next_out = (unsigned char*)bytebuf;
        gz.avail_out = sizeof(bytebuf);
        r = deflate(&gz, flush);
        if (r == Z_BUF_ERROR)
        {
            flush = Z_FINISH;
        }
        else if (r != Z_OK && r != Z_STREAM_END)
        {
            throw std::runtime_error(std::string("Could not deflate buffer: ") + gz.msg);
        }
        brg::span<const brg::byte> outspan(bytebuf, sizeof(bytebuf) - gz.avail_out);
        vec.insert(vec.end(), outspan.begin(), outspan.end());
    } while (r != Z_STREAM_END);

    writeOut(brg::span<const brg::byte>(vec.data(), vec.size()));

    deflateEnd(&gz);
}

void brg::unzip(brg::span<const brg::byte> data, const std::function<void(brg::span<const brg::byte>)>& writeOut)
{
    auto* buf = data.begin();
    auto sz = data.size();

    z_stream gz;
    gz.zalloc = Z_NULL;
    gz.zfree = Z_NULL;
    gz.opaque = Z_NULL;

    gz.next_in = (unsigned char*)buf;
    gz.avail_in = sz;
    inflateInit2(&gz, MAX_WBITS + 32);

    std::vector<brg::byte> vec;
    char bytebuf[16*1024];
    int inflateResult;
    do
    {
        gz.next_out = (unsigned char*)bytebuf;
        gz.avail_out = sizeof(bytebuf);
        inflateResult = inflate(&gz, Z_NO_FLUSH);
        if (inflateResult != Z_OK && inflateResult != Z_STREAM_END)
        {
            throw std::runtime_error(std::string("Could not inflate buffer: ") + gz.msg);
        }
        brg::span<const brg::byte> outspan(bytebuf, sizeof(bytebuf) - gz.avail_out);
        vec.insert(vec.end(), outspan.begin(), outspan.end());
    } while (inflateResult != Z_STREAM_END);
    writeOut(brg::span<const brg::byte>(vec.data(), vec.size()));
    inflateEnd(&gz);
}
