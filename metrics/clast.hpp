#pragma once

#include <string>
#include <set>
#include <vector>
#include <map>
#include <clang-c/Index.h>


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


class XmlWriter;


class Clast
{
    public:
        static CXChildVisitResult visit_children(CXCursor cursor, CXCursor, CXClientData data);

        VisitorData* getData();

        void report(std::ostream &) const;

    private:
        Node root{{}, "project"};
        std::map<std::string, Node*> visited{};
        VisitorData data{&root, &visited};

        void write(const Node *node, XmlWriter&) const;
};


}
