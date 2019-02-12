#pragma once

#include <string>
#include <map>
#include <set>
#include <ostream>
#include <istream>
#include <vector>
#include <memory>
#include <tinyxml2.h>


namespace graphviz
{


typedef std::vector<std::string> NodeName;

NodeName operator+(const NodeName&, const std::string&);


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


struct Edge
{
    NodeName source;
    NodeName destination;
    std::string description;
};

struct Node
{
    NodeName name;
};

bool operator<(const Node&, const Node&);


class Graph
{
  public:
    void addNode(const NodeName&);
    void addEdge(const NodeName&, const NodeName&, const std::string& description = "");
    void writeTo(Writer&) const;

    void squashEdges();

    void serialize(std::ostream&) const;
    void load(std::istream&);

    void load(const tinyxml2::XMLDocument&);

  private:
    std::set<Node> nodes{};
    std::vector<Edge> edges{};

    void load(const tinyxml2::XMLElement*, const NodeName& path, std::map<std::string, NodeName> &idMap, std::map<std::string, std::set<std::string> > &links);

};


}
