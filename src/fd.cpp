#include "fd.h"

#include <unistd.h>

void brg::FileD::reset()
{
    if (m_fd != -1)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

const brg::FileD& brg::operator<<(const FileD& fd, const span<const byte>& data)
{
    auto buf = data.begin();
    auto bytesToWrite = data.size();

    std::size_t bytesWritten = 0;
    while (bytesWritten < bytesToWrite)
    {
        bytesWritten += ::write(fd.handle(), buf + bytesWritten, bytesToWrite - bytesWritten);
    }
    return fd;
}

brg::Pipe brg::makePipe()
{
    int fd[2];
    ::pipe(fd);
    return Pipe{FileD::fromHandle(fd[0]), FileD::fromHandle(fd[1])};
}
