#include "Visitor.hpp"
#include <clang-c/Index.h>


CXChildVisitResult Visitor::visitor_recursive(
    CXCursor cursor,
    CXCursor parent,
    CXClientData data)
{
  Visitor* visitor = static_cast<Visitor *>(data);
  return visitor->visit(cursor, parent);
}

bool Visitor::ignore(CXCursor cursor) const
{
  const auto location = clang_getCursorLocation(cursor);
  return clang_Location_isInSystemHeader(location);
}

