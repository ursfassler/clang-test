#pragma once

#include "Visitor.hpp"
#include "graphviz.hpp"

namespace metric
{


struct Node
{
        Node(const std::string& name_, const std::string& type_) :
            name{name_},
            type{type_}
        {
        }

        std::string name{};
        std::string type{};
        std::string file{};
        unsigned line{};
        std::set<std::string> references{};

        std::vector<Node*> children{};
};


struct VisitorData
{
        Node* parent;
        std::map<std::string, Node*>* visited;
};


class Clast :
    public Visitor
{
  private:
    Node root{{}, {}};
    std::map<std::string, Node*> visited{};

    static CXChildVisitResult visit_children(CXCursor cursor, CXCursor, CXClientData data);
  public:
    std::string name() const override;

    const graphviz::Graph& graph() const override;

    CXChildVisitResult visit(
        CXCursor cursor,
        CXCursor parent) override;

    void report(std::ostream &) const override;
};


}
