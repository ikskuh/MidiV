#ifndef HAL_HPP
#define HAL_HPP

#include <string>

namespace HAL
{
    std::string GetWorkingDirectory();

    bool SetWorkingDirectory(std::string const & dir);

    std::string GetFullPath(std::string const & path, ptrdiff_t * nameOffset = nullptr);

	std::string GetDirectoryOf(std::string const & file);
}

#endif // HAL_HPP
