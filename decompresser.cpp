#include "src/crypt.h"
#include "src/fd.h"
#include "src/tar_reader.h"
#include "src/zip.h"

#include <dlfcn.h> // dlsym
#include <thread>

extern "C" FILE* fopen64(const char* name, const char* mode)
{
    static brg::TarReader tr(::getenv("VIRTUAL_TAR_FS"));
    if (auto fileData = tr.fileData(name))
    {
        auto pipe = brg::makePipe();

        std::thread([wFd = std::move(pipe.in), fileData] ()
        {
            using bytespan = brg::span<const brg::byte>;

            brg::decrypt(fileData, [&wFd] (bytespan data) {
                brg::unzip(data, [&wFd] (bytespan data) {
                    wFd << data;
                });
            });

        }).detach();

        return fdopen(pipe.out.release(), mode);
    }
    else
    {
        using FOpen64Proto = FILE*(*)(const char*, const char*);
        static auto fopen64Sym = reinterpret_cast<FOpen64Proto>(dlsym(RTLD_NEXT, "fopen64"));
        return fopen64Sym(name, mode);
    }
}
