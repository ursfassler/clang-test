#include "graphviz.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


int main(int argc, char* argv[])
{
  std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 2) {
    std::cerr << "expected filename" << std::endl;
    return -1;
  }

  graphviz::Graph graph;

  boost::property_tree::ptree pt;
  boost::property_tree::read_json(arg[1], pt);
  graph.load(pt);

  graph.squashEdges();

  {
    std::ofstream file{arg[1] + ".gv"};
    graphviz::Writer writer{file};
    graph.writeTo(writer);
  }

  return 0;
}
