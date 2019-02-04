#pragma once

#include "Visitor.hpp"
#include "graphviz.hpp"

namespace metric
{


class Dependency :
    public Visitor
{
  private:
    void collect_references(CXCursor clazz, CXCursor root);

    static CXChildVisitResult collect_references(
        CXCursor cursor,
        CXCursor parent,
        CXClientData data);

    graphviz::Graph bgraph{};

    bool isInProject(CXCursor) const;

  public:
    std::string name() const override;

    const graphviz::Graph& graph() const override;

    CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) override;

    void report(std::ostream &) const override;
};


}
