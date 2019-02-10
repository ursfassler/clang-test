#include "XmlWriter.hpp"


namespace metric
{


XmlWriter::XmlWriter(std::ostream& stream_) :
  stream{stream_}
{
}

void XmlWriter::startNode(const std::string& name)
{
  if (nodeIsOpen) {
    stream << ">";
    stream << std::endl;
    nodeIsOpen = false;
  }

  stream << std::string(path.size() * 4, ' ');
  stream << "<" << escape(name);
  path.push_back(name);

  nodeIsOpen = true;
}

void XmlWriter::endNode()
{
  if (nodeIsOpen) {
    path.pop_back();
    stream << "/>";
    stream << std::endl;
    nodeIsOpen = false;
  } else {
    const auto name = path.back();
    path.pop_back();
    stream << std::string(path.size() * 4, ' ');
    stream << "</" << escape(name) << ">";
    stream << std::endl;
  }
}

void XmlWriter::attribute(const std::string& name, const std::string& value)
{
  stream << " " << escape(name) <<  "=\"" << escape(value) << "\"";
}

std::string XmlWriter::escape(const std::string& value)
{
  std::string result;

  for (const auto& sym : value) {
    switch (sym) {
      case '"':
        result += "&quot;";
        break;
      case '\'':
        result += "&apos;";
        break;
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '&':
        result += "&amp;";
        break;

      default:
        result += sym;
    }
  }

  return result;
}


}
