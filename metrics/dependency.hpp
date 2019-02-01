#pragma once

#include "Visitor.hpp"
#include <map>
#include <set>
#include <vector>

class Dependency :
    public Visitor
{
  private:
    void collect_references(CXCursor clazz, CXCursor root);

    static CXChildVisitResult collect_references(
        CXCursor cursor,
        CXCursor parent,
        CXClientData data);

    typedef std::vector<std::string> Path;
    std::map<Path, std::set<Path>> graph{};

    bool isInProject(CXCursor) const;

  public:
    CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) override;

    void report(std::ostream &) const override;
};
