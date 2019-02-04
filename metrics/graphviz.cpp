#include "graphviz.hpp"

#include <sstream>


namespace graphviz
{
namespace
{


std::string escape(const std::string& value)
{
  std::string result;

  result += "_";

  for (const auto& sym : value) {
    switch (sym) {
      case '_':
        result += "_5f";
        break;
      case ':':
        result += "_3a";
        break;
      case '=':
        result += "_3d";
        break;
      case '[':
        result += "_5b";
        break;
      case ']':
        result += "_5d";
        break;
      case '<':
        result += "_3c";
        break;
      case '>':
        result += "_3e";
        break;
      case '!':
        result += "_21";
        break;
      case '*':
        result += "_2a";
        break;
      case '+':
        result += "_2b";
        break;
      case '-':
        result += "_2d";
        break;
      case '.':
        result += "_2e";
        break;

      default:
        result += sym;
    }
  }

  return result;
}

std::string serialize(const NodeName& value)
{
  std::string result{};

  bool first = true;
  for (const auto& itr : value) {
    if (first) {
      first = false;
    } else {
      result += "::";
    }

    result += itr;
  }

  return result;
}

template <class Container>
void split3(const std::string& str, Container& cont, const std::string& delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + delim.size();
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

NodeName deserialize(const std::string& value)
{
  NodeName result;
  split3(value, result, "::");
  return result;
}


}


Writer::Writer(std::ostream& stream) :
  os{stream}
{
}

void Writer::start()
{
  os << "digraph" << std::endl;
  os << "{" << std::endl;
  os << "rankdir=\"LR\";" << std::endl;
  os << "node[shape = box];" << std::endl;
}

void Writer::startSubgraph(const NodeName& name, unsigned number)
{
  os << "subgraph cluster_" << number << std::endl;
  os << "{" << std::endl;
  os << "label = \"" << serialize(name) << "\";" << std::endl;
}

void Writer::end()
{
  os << "}" << std::endl;
}

void Writer::node(const NodeName& name)
{
  const auto serialized = serialize(name);
  os << escape(serialized) << " [label=\"" + serialized + "\"]" <<  std::endl;
}

void Writer::edge(const NodeName& source, const NodeName& destination)
{
  os << escape(serialize(source)) << " -> " << escape(serialize(destination)) << std::endl;
}

void Writer::separate()
{
  os << std::endl;
}



void Graph::addNode(const NodeName& value)
{
  nodes.insert(value);
}

void Graph::addEdge(const NodeName& source, const NodeName& destination, const std::string &description)
{
  addNode(source);
  addNode(destination);

  edges.push_back({source, destination, description});
}

void Graph::writeTo(Writer& writer) const
{
  Tree tree{};
  writer.start();
  writer.separate();

  for (const auto& itr : nodes) {
    tree.add(itr);
  }
  tree.writeTo(writer);
  writer.separate();

  for (const auto& itr : edges) {
    writer.edge(itr.source, itr.destination);
  }

  writer.end();
}

void Graph::serialize(std::ostream& stream) const
{
  for (const auto& itr : nodes) {
    stream << graphviz::serialize(itr);
    stream << std::endl;
  }

  stream << std::endl;

  for (const auto& itr : edges) {
    stream << graphviz::serialize(itr.source);
    stream << " ";
    stream << graphviz::serialize(itr.destination);
    stream << " ";
    stream << itr.description;
    stream << std::endl;
  }
}

void Graph::load(std::istream& stream)
{
  nodes.clear();
  edges.clear();

  std::string line;
  while (std::getline(stream, line)) {
    if (line == "") {
      break;
    }

    const auto node = deserialize(line);
    addNode(node);
  }

  while (!stream.eof()) {
    std::string sourceName;
    std::string destinationName;
    stream >> sourceName;
    stream >> destinationName;

    if (sourceName == "") {
      break;
    }

    addEdge(deserialize(sourceName), deserialize(destinationName));
  }
}

void arrayAdd(boost::property_tree::ptree& array, const boost::property_tree::ptree& value)
{
  boost::property_tree::ptree::value_type node{"", value};
  array.push_back(node);
}

boost::property_tree::ptree writeName(const NodeName& value)
{
  boost::property_tree::ptree array;
  for (const auto& itr : value) {
    boost::property_tree::ptree::value_type node{"", itr};
    array.push_back(node);
  }
  return array;
}

void Graph::serialize(boost::property_tree::ptree& tree) const
{
  {
    boost::property_tree::ptree sn;
    for (const auto& itr : nodes) {
      boost::property_tree::ptree node;
      node.add_child("name", writeName(itr));
      arrayAdd(sn, node);
    }
    tree.add_child("nodes", sn);
  }
  {
    boost::property_tree::ptree sn;
    for (const auto& itr : edges) {
      boost::property_tree::ptree node;
      node.add_child("source", writeName(itr.source));
      node.add_child("destination", writeName(itr.destination));
      if (!itr.description.empty()) {
        node.add("description", itr.description);
      }
      arrayAdd(sn, node);
    }
    tree.add_child("edges", sn);
  }
}

graphviz::NodeName readName(const boost::property_tree::ptree& value)
{
  graphviz::NodeName name{};
  for (const auto part : value) {
    name.push_back(part.second.data());
  }
  return name;
}

void Graph::load(const boost::property_tree::ptree& root)
{
  nodes.clear();
  edges.clear();

  const auto nodes = root.get_child("nodes");
  for (const auto node : nodes) {
    const auto name = readName(node.second.get_child("name"));
    addNode(name);
  }

  const auto edges = root.get_child("edges");
  for (const auto edge : edges) {
    const auto source = readName(edge.second.get_child("source"));
    const auto destination = readName(edge.second.get_child("destination"));
    addEdge(source, destination);
  }
}

void Tree::add(const NodeName& value)
{
  TreeNode* node = &root;
  for (const auto& part : value) {
    node = &node->children[part];
  }
  node->print = true;
}

void Tree::writeTo(Writer& writer)
{
  unsigned number = 0;
  writeTo(root, {}, number, writer);
}

void Tree::writeTo(const TreeNode& node, const NodeName& path, unsigned& number, Writer& writer)
{
  for (const auto& child : node.children) {
    NodeName np = path;
    np.push_back(child.first);

    if (child.second.print) {
      writer.node(np);
    }

    if (!child.second.children.empty()) {
      writer.startSubgraph(np, number);
      number++;
      writeTo(child.second, np, number, writer);
      writer.end();
      writer.separate();
    }
  }
}


}
