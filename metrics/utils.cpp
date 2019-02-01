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


}
