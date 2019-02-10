#pragma once

#include "graphviz.hpp"
#include <clang-c/Index.h>
#include <string>
#include <ostream>
#include <map>
#include <vector>


class Visitor
{
  public:
    static bool ignore(CXCursor cursor);

  public:
    virtual ~Visitor() = default;

    virtual std::string name() const = 0;

    virtual CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) = 0;

    virtual const graphviz::Graph& graph() const = 0;
    virtual void report(std::ostream &) const = 0;

    static CXChildVisitResult visitor_recursive(
        CXCursor cursor,
        CXCursor parent,
        CXClientData data);
};
