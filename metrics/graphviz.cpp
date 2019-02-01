#include "graphviz.hpp"


namespace graphviz
{


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

std::string Writer::escape(const std::string& value)
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

std::string Writer::serialize(const NodeName& value)
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


void Graph::addNode(const NodeName& value)
{
  edges[value];
}

void Graph::addEdge(const NodeName& source, const NodeName& destination)
{
  addNode(source);
  addNode(destination);

  edges[source].insert(destination);
}

void Graph::writeTo(Writer& writer)
{
  Tree tree{};
  writer.start();
  writer.separate();

  for (const auto& itr : edges) {
//    writer.node(itr.first);
    tree.add(itr.first);
  }
  tree.writeTo(writer);
  writer.separate();

  for (const auto& itr : edges) {
    for (const auto& dest : itr.second) {
      writer.edge(itr.first, dest);
    }
  }

  writer.end();
}

void Tree::add(const NodeName& value)
{
  TreeNode* node = &root;
  for (const auto& part : value) {
    node = &node->children[part];
  }
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

    if (child.second.isLeaf()) {
      writer.node(np);
    } else {
      writer.startSubgraph(np, number);
      number++;
      writeTo(child.second, np, number, writer);
      writer.end();
      writer.separate();
    }
  }
}

bool TreeNode::isLeaf() const
{
  return children.empty();
}


}
