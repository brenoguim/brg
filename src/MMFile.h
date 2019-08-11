#ifndef BRG_MMFILE_H
#define BRG_MMFILE_H

#include "defs.h"

namespace brg
{

class MMFile
{
  public:
    MMFile(const char* fileName);

    MMFile() = delete;
    MMFile(const MMFile&) = delete;
    MMFile& operator=(const MMFile&) = delete;
    MMFile(MMFile&&) = delete;
    MMFile& operator=(MMFile&&) = delete;
    ~MMFile();

    const byte* begin() const { return m_data.begin(); }
    const byte* end() const { return m_data.end(); }

    operator span<const byte>() const { return span<const byte>(begin(), m_data.size()); }

  private:
    span<byte> m_data;
};

std::size_t getFileSize(const char*);

}

#endif
