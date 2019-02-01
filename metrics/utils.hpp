#pragma once

#include <clang-c/Index.h>
#include <string>
#include <vector>


namespace utils
{


typedef std::vector<std::string> Path;

Path getPath(CXCursor);


}
