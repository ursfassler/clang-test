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



class XmlWriter
{
    public:
        XmlWriter(std::ostream&);

        void startNode(const std::string&);
        void endNode();
        void attribute(const std::string& name, const std::string& value);

    private:
        std::ostream& stream;
        std::vector<std::string> path{};
        bool nodeIsOpen{false};

        std::string escape(const std::string &value);
};


class Clast
{
  private:
        Node root{{}, "project"};
    std::map<std::string, Node*> visited{};
    VisitorData data{&root, &visited};

    void write(const Node *node, XmlWriter&) const;
    public:
    static CXChildVisitResult visit_children(CXCursor cursor, CXCursor, CXClientData data);

    VisitorData* getData();

    void report(std::ostream &) const;
};


}
