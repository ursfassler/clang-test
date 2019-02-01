#pragma once

#include "Visitor.hpp"
#include "VisitorDescriptor.hpp"
#include <map>
#include <set>
#include <vector>

class VisitorFactory;

class Dependency :
    public Visitor
{
  private:
    static const VisitorDescriptor DESCRIPTOR;

    void collect_references(CXCursor clazz, CXCursor root);

    static CXChildVisitResult collect_references(
        CXCursor cursor,
        CXCursor parent,
        CXClientData data);

    typedef std::vector<std::string> Path;
    std::map<Path, std::set<Path>> graph{};

    bool isInProject(CXCursor) const;

  public:
    const std::string & get_name() const override;
    const std::string & get_id() const override;

    CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) override;

    void report(std::ostream &) const override;

    static void register_in(VisitorFactory &);
};
