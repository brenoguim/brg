#include "zip.h"

#include <stdexcept>
#include <string>
#include <zlib.h>
#include <iostream>

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

    char bytebuf[16*1024];
    do
    {
        gz.next_out = (unsigned char*)bytebuf;
        gz.avail_out = sizeof(bytebuf);
        deflate(&gz, Z_FINISH);
        if (sizeof(bytebuf) == gz.avail_out)
        {
            throw std::runtime_error(std::string("Could not deflate buffer: ") + gz.msg);
        }
        writeOut(brg::span<const brg::byte>(bytebuf, sizeof(bytebuf) - gz.avail_out));
    } while (gz.avail_in);
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

    char bytebuf[16*1024];
    do
    {
        gz.next_out = (unsigned char*)bytebuf;
        gz.avail_out = sizeof(bytebuf);
        inflate(&gz, Z_NO_FLUSH);
        if (sizeof(bytebuf) == gz.avail_out)
        {
            throw std::runtime_error(std::string("Could not inflate buffer: ") + gz.msg);
        }
        writeOut(brg::span<const brg::byte>(bytebuf, sizeof(bytebuf) - gz.avail_out));
    } while (gz.avail_in);
    inflateEnd(&gz);
}
