#pragma once

#define STR(x) #x
#define STRSTR(x) STR(x)
#define die(_msg) _die("ded in " __FILE__ ":" STRSTR(__LINE__), _msg)
[[noreturn]] void _die(char const * context, char const * msg);
