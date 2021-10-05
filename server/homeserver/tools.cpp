#include "common.hpp"
#include "tools.hpp"

std::string PrettyBytes(size_t bytes)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2);

	if (bytes >= 1000000000000ull)
		ss << bytes / 1000000000000.0 << "TB";
	else if (bytes >= 1000000000ull)
		ss << bytes / 1000000000.0 << "GB";
	else if (bytes >= 1000000ull)
		ss << bytes / 1000000.0 << "MB";
	else if (bytes >= 1000ull)
		ss << bytes / 1000.0 << "KB";
	else
		ss << bytes << "B";

	return ss.str();
}
