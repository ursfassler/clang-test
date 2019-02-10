#include "utils.hpp"

#include "Clang.hpp"
#include <algorithm>


namespace utils
{


Path getPath(CXCursor cursor)
{
  Path result{};

  for (;;) {
    result.push_back(Clang::getCursorSpelling(cursor));

    cursor = clang_getCursorSemanticParent(cursor);
    CXCursorKind kind = Clang::getCursorKind(cursor);

    if (kind == CXCursor_TranslationUnit) {
      break;
    }
  }

  std::reverse(result.begin(), result.end());

  return result;
}

std::string serialize(const Path& value)
{
  std::string result{};

  bool first = true;
  for (const auto& itr : value) {
    if (first) {
      first = false;
    } else {
      result += "::";
    }

    result += itr;
  }

  return result;
}

std::string location(CXCursor value)
{
  const auto sl = clang_getCursorLocation(value);
  CXFile file;
  unsigned int line;
  clang_getFileLocation(sl, &file, &line, nullptr, nullptr);
  const auto cxfilename = clang_getFileName(file);
  const std::string filename = Clang::to_string(cxfilename);

  return filename + ":" + std::to_string(line);
}


}
