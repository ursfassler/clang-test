#include "graphviz.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char* argv[])
{
  std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 2) {
    std::cerr << "expected filename" << std::endl;
    return -1;
  }

  graphviz::Graph graph;

  {
    std::ifstream file{arg[1]};
    graph.load(file);
  }

  {
    std::ofstream file{arg[1] + ".gv"};
    graphviz::Writer writer{file};
    graph.writeTo(writer);
  }

  return 0;
}
