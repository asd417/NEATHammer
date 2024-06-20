#pragma once
#include <string>
#include <cstdarg>
#include <vector>
#include <string>

namespace Strutil {
	std::string vformat(const char* fmt, va_list ap);
	std::string format(const char* fmt, ...);
}