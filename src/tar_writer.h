#ifndef BRG_TAR_WRITER
#define BRG_TAR_WRITER

#include "defs.h"
#include "serial.h"

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace brg
{

class TarWriter
{
    struct FileData
    {
        std::string name;
        std::size_t begin;
    };
  public:
    TarWriter(const std::string& outputFile)
        : m_output(outputFile, std::ofstream::out | std::ofstream::binary)
    {}

    ~TarWriter()
    {
        finish();
    }

    void startFile(const std::string& fileName)
    {
        m_files.push_back(FileData{fileName, m_currentPos});
    }

    void write(brg::span<const byte> data)
    {
        m_output.write(data.begin(), data.size());
        m_currentPos += data.size();
    }

    void finish()
    {
        if (!m_files.empty())
        {
            auto itLast = m_files.begin();
            for (auto it = std::next(itLast); it != m_files.end(); ++it, ++itLast)
            {
                writeFileInformation(*itLast, it->begin - itLast->begin);
            }
            writeFileInformation(*itLast, m_currentPos - itLast->begin);
        }
        // Write number of files
        serializeForBackward(m_output, m_files.size());
    }

  private:
    void writeFileInformation(const FileData& file2data, std::size_t dataSize)
    {
        // Write file name
        serializeForBackward(m_output, file2data.name);
        // Write file size
        serializeForBackward(m_output, dataSize);
        // Write file offset 
        serializeForBackward(m_output, file2data.begin);
    }

    std::ofstream m_output;
    std::size_t m_currentPos {0};

    std::vector<FileData> m_files;
};

}

#endif
