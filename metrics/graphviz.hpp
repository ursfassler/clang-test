#pragma once

#include <string>
#include <map>
#include <set>
#include <ostream>
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

    std::string escape(const std::string&);
    std::string serialize(const NodeName&);
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
    void addEdge(const NodeName&, const NodeName&);
    void writeTo(Writer&) const;

  private:
    std::map<NodeName, std::set<NodeName>> edges{};
};


}
