#pragma once

#include <string>
#include <map>
#include <set>
#include <ostream>
#include <istream>
#include <vector>
#include <memory>


namespace graphviz
{


typedef std::vector<std::string> NodeName;


class Writer
{
  public:
    Writer(std::ostream&);

    void start();
    void startSubgraph(const NodeName&, unsigned number);
    void end();

    void node(const NodeName&);
    void edge(const NodeName&, const NodeName&);
    void separate();

  private:
    std::ostream& os;

};


struct TreeNode
{
    bool print{false};
    std::map<std::string, TreeNode> children;
};

class Tree
{
  public:
    void add(const NodeName&);

    void writeTo(Writer&);

  private:
    TreeNode root{};

    void writeTo(const TreeNode&, const NodeName&, unsigned& number, Writer&);

};


class Graph
{
  public:
    void addNode(const NodeName&);
    void addEdge(const NodeName&, const NodeName&, const std::string& description = "");
    void writeTo(Writer&) const;

    void serialize(std::ostream&) const;
    void load(std::istream&);

  private:
    std::set<NodeName> nodes{};

    struct Edge
    {
        NodeName source;
        NodeName destination;
        std::string description;
    };

    std::vector<Edge> edges{};
};


}
