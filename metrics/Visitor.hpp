#ifndef __VISITOR__HPP__
#define __VISITOR__HPP__

#include <clang-c/Index.h>
#include <string>
#include <ostream>
#include <map>
#include <vector>

class Visitor
{
  public:
    bool ignore(CXCursor cursor) const;

  public:
    virtual ~Visitor() = default;

    virtual const std::string & get_name() const = 0;
    virtual const std::string & get_id() const = 0;

    virtual CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) = 0;

    virtual void report(std::ostream &) const = 0;

    static CXChildVisitResult visitor_recursive(
        CXCursor cursor,
        CXCursor parent,
        CXClientData data);
};


#endif
