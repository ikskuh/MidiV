#ifndef HAL_HPP
#define HAL_HPP

#include <string>

namespace HAL
{
    std::string GetWorkingDirectory();

    bool SetWorkingDirectory(std::string const & dir);

    std::string GetFullPath(std::string const & path, ptrdiff_t * nameOffset = nullptr);
}

#endif // HAL_HPP
