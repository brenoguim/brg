#include "src/MMFile.h"
#include "src/tar_writer.h"
#include "src/zip.h"
#include "src/crypt.h"

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    bool gz = false;
    bool crypt = true;
    std::string outputFile;

    if (argc == 2)
    {
        outputFile = argv[1];
    }
    else if (argc == 3)
    {
        if (std::string(argv[1]) != "-z")
        {
            throw std::runtime_error("Only supported switch is -z");
        }
        gz = true;
        outputFile = argv[2];
    }
    else
    {
        throw std::runtime_error("Bad syntax");
    }

    brg::TarWriter twriter(outputFile);

    using bytespan = brg::span<const brg::byte>;
    std::function<void(bytespan)> addDataToTar = [&twriter] (bytespan data)
    {
        twriter.write(data);
    };

    if (crypt)
    {
        addDataToTar = [innerFn = addDataToTar] (bytespan data)
        {
            brg::encrypt(33, data, innerFn);
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
