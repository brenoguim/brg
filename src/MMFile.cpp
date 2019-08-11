#include "MMFile.h"

#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

brg::MMFile::MMFile(const char* fileName)
{
    auto sz = getFileSize(fileName);
    int fd = ::open(fileName, O_RDONLY, 0);
    auto begin = reinterpret_cast<byte*>(::mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0));
    m_data = span<byte>(begin, sz);
    ::close(fd);
}

brg::MMFile::~MMFile()
{
    ::munmap(m_data.begin(), m_data.size());
}

std::size_t brg::getFileSize(const char* fileName)
{
    struct stat stat_buf;
    int rc = ::stat(fileName, &stat_buf);
    if (rc != 0)
    {
        throw std::runtime_error(std::string("Cannot stat file ") + fileName);
    }
    return stat_buf.st_size;
}
