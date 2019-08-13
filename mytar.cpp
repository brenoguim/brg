#include "src/MMFile.h"
#include "src/tar_writer.h"
#include "src/zip.h"
#include "src/scramble.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    bool gz = false;
    bool scramble = false;
    const char* scrambleKey = nullptr;
    std::string outputFile;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-z")
        {
            gz = true;
        }
        else if (arg == "-k")
        {
            scramble = true;
            scrambleKey = argv[++i];
        }
        else if (arg == "-o")
        {
            outputFile = argv[++i];
        }
        else
        {
            throw std::runtime_error("Unexpected argument: " + arg);
        }
    }

    if (outputFile.empty())
    {
        throw std::runtime_error("No output file specified.");
    }

    brg::TarWriter twriter(outputFile);

    using bytespan = brg::span<const brg::byte>;
    std::function<void(bytespan)> addDataToTar = [&twriter] (bytespan data)
    {
        twriter.write(data);
    };

    if (scramble)
    {
        addDataToTar = [scrambleKey, innerFn = addDataToTar] (bytespan data)
        {
            brg::scramble(scrambleKey, data, innerFn);
        };
    }

    if (gz)
    {
        addDataToTar = [innerFn = addDataToTar] (bytespan data)
        {
            brg::zip(data, innerFn);
        };
    }

    std::string fileName;
    while (std::cin >> fileName)
    {
        twriter.startFile(fileName);
        brg::MMFile mmfile(fileName.c_str());
        addDataToTar(mmfile);
    }
}
