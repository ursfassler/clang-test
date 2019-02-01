#include "Visitor.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include <vector>
#include <algorithm>

Visitor * visitor_cast(CXClientData data)
{
  return static_cast<Visitor *>(data);
}

CXChildVisitResult Visitor::visitor_recursive(
    CXCursor cursor,
    CXCursor parent,
    CXClientData data)
{
  return visitor_cast(data)->visit(cursor, parent);
}

bool Visitor::ignore(CXCursor cursor) const
{
  return Location(cursor).is_in_system_header();
}

