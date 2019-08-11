#ifndef BRG_TAR_READER_H
#define BRG_TAR_READER_H

#include "MMFile.h"
#include "defs.h"
#include "serial.h"

#include <unordered_map>
#include <vector>
#include <iostream>

namespace brg
{

class TarReader
{
  public:
    TarReader(const char* fileName)
        : m_tar(fileName)
    {
        auto buf = m_tar.end();
        auto numFiles = brg::deserializeBackwards<std::size_t>(buf);

        struct FileData
        {
            std::string name;
            std::size_t offset, size;
        };
        std::vector<FileData> files;
        files.reserve(numFiles);

        for (std::size_t i = 0; i < numFiles; ++i)
        {
            auto fileOffset = brg::deserializeBackwards<std::size_t>(buf);
            auto fileSize = brg::deserializeBackwards<std::size_t>(buf);
            auto fileNameSize = brg::deserializeBackwards<std::size_t>(buf);
            auto fileName = brg::deserializeBackwards<std::string>(buf, fileNameSize);
            files.emplace_back(FileData{std::move(fileName), fileOffset, fileSize});
        }

        for (auto& fileData : files)
        {
            m_files[std::move(fileData.name)] = span<const byte>(m_tar.begin() + fileData.offset, fileData.size);
        }
    }

    span<const byte> fileData(const std::string& fileName) const
    {
        auto it = m_files.find(fileName);
        return it != m_files.end() ? it->second : span<const byte>();
    }

  private:
    brg::MMFile m_tar;
    std::unordered_map<std::string, span<const byte>> m_files; 
};

}

#endif
