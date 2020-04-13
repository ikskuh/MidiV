#pragma once

#include <sstream>
#include <iostream>

struct Log
{
    std::stringstream stream;

    Log() = default;
    Log(Log const &) = delete;
    Log(Log &&) = delete;
    ~Log()
    {
        std::cerr << stream.str() << std::endl;
        std::cerr.flush();
    }
};

template<typename T>
Log & operator << (Log & log, T const & value)
{
    log.stream << value;
    return log;
}

template<typename T>
auto && operator << (Log && log, T const & value)
{
    log.stream << value;
    return log;
}
