#include "hal.hpp"

#ifdef MIDIV_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::string HAL::GetWorkingDirectory()
{
    char path[MAX_PATH + 1];
    DWORD length = GetCurrentDirectoryA(sizeof path, path);
    if(length == 0)
        return std::string();
    return std::string(path, length);
}

bool HAL::SetWorkingDirectory(std::string const & dir)
{
    return (SetCurrentDirectoryA(dir.c_str()) == TRUE);
}

std::string HAL::GetFullPath(std::string const & relpath, ptrdiff_t * nameOffset)
{
    char path[MAX_PATH + 1];
    char * offset;
    DWORD length = GetFullPathNameA(relpath.c_str(), sizeof path, path, &offset);
    if(length == 0)
    {
        if(nameOffset != nullptr)
            *nameOffset = 0;
        return std::string();
    }
    else
    {
        if(nameOffset != nullptr)
            *nameOffset = offset - path;
        return std::string(path, length);
    }
}

#endif
