#include "hal.hpp"

std::string HAL::GetDirectoryOf(std::string const & file)
{
	ptrdiff_t offset;
    auto fullPath = HAL::GetFullPath(file, &offset);
    return fullPath.substr(0, size_t(offset));
}

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

#ifdef MIDIV_LINUX

#include <unistd.h>
#include <limits.h>
#include <string.h>

std::string HAL::GetWorkingDirectory()
{
	char buf[PATH_MAX + 1];
	return std::string(getcwd(buf, sizeof buf));
}

bool HAL::SetWorkingDirectory(std::string const & dir)
{
    return (0 == chdir(dir.c_str()));
}

std::string HAL::GetFullPath(std::string const & relpath, ptrdiff_t * nameOffset)
{
	char buf[PATH_MAX + 1];
	if(realpath(relpath.c_str(), buf) == nullptr)
		return std::string();
	char * pos = strrchr(buf, '/');
	if(pos != nullptr && nameOffset != nullptr)
		*nameOffset = pos - buf;
	return std::string(buf);
}

#endif
