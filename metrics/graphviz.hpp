#pragma once

#include <boost/property_tree/ptree.hpp>
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


class GraphWriter
{
  public:
    virtual void node(const Node&) = 0;
    virtual void edge(const Edge&) = 0;

};

class JsonWriter :
    public GraphWriter
{
  public:
    void node(const Node&) override;
    void edge(const Edge&) override;
    void writeFile(const std::string&) const;

  private:
    boost::property_tree::ptree nodes{};
    boost::property_tree::ptree edges{};

};

class Graph
{
  public:
    void addNode(const NodeName&);
    void addEdge(const NodeName&, const NodeName&, const std::string& description = "");
    void writeTo(Writer&) const;

    void squashEdges();

    void serialize(GraphWriter&) const;

    void serialize(std::ostream&) const;
    void load(std::istream&);

    void load(const boost::property_tree::ptree&);

  private:
    std::set<Node> nodes{};
    std::vector<Edge> edges{};
};


}
