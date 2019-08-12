#ifndef BRG_FD_H
#define BRG_FD_H

#include "defs.h"

namespace brg
{

class FileD
{
    explicit FileD(int fd) : m_fd(fd) {}

  public:
    static FileD fromHandle(int fd) { return FileD(fd); }

    FileD() = default;

    FileD(const FileD&) = delete;
    FileD& operator=(const FileD&) = delete;

    FileD(FileD&& other)
        : m_fd(other.m_fd)
    {
        other.m_fd = -1;
    }

    FileD& operator=(FileD&& other)
    {
        reset();
        m_fd = other.m_fd;
        other.m_fd = -1;
        return *this;
    }

    ~FileD() { reset(); }

    void reset();

    int handle() const { return m_fd; }
    int release()
    {
        auto fd = m_fd;
        m_fd = -1;
        return fd;
    }

  private:
    int m_fd {-1};
};

const FileD& operator<<(const FileD&, const span<const byte>&);

struct Pipe
{
    FileD out, in;
};
Pipe makePipe();

}

#endif
