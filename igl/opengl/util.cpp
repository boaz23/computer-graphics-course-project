#include "util.h"

#ifdef WIN32
#include <windows.h>
#elif UNIX
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#else
#endif

std::string CanonicalizePath(const std::string& path, bool* success)
{
#ifdef WIN32
    char canonicalPath[MAX_PATH];
    DWORD n = GetFullPathName(path.c_str(), MAX_PATH, canonicalPath, nullptr);
    if (n == 0 || n > MAX_PATH)
    {
        *success = false;
    }
    else
    {
        return std::string{ canonicalPath , n };
    }
#elif UNIX
    char *canonicalPath = canonicalize_file_name(path);
    if (canonicalPath == NULL)
    {
        *success = false;
    }
    else
    {
        return std::string{ canonicalPath };
    }
#else
    throw std::exception("Unsupported platform");
#endif
    return path;
}
