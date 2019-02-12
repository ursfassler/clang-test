#include "graphviz.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <tinyxml2.h>


int main(int argc, char* argv[])
{
  std::vector<std::string> arg{argv, argv+argc};

  if (arg.size() != 2) {
    std::cerr << "expected filename" << std::endl;
    return -1;
  }

  graphviz::Graph graph;

  tinyxml2::XMLDocument xml_doc;

  tinyxml2::XMLError eResult = xml_doc.LoadFile(arg[1].c_str());
  if (eResult != tinyxml2::XML_SUCCESS)
    return false;

  graph.load(xml_doc);

  graph.squashEdges();

  {
    std::ofstream file{arg[1] + ".gv"};
    graphviz::Writer writer{file};
    graph.writeTo(writer);
  }

  return 0;
}
