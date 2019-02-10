#include "clast.hpp"
#include "Clang.hpp"
#include "utils.hpp"
#include <vector>
#include <ostream>


namespace metric
{


std::map<CXCursorKind, std::string> KindName
{
  {CXCursor_Namespace, "namespace"},
  {CXCursor_TypedefDecl, "class"},
  {CXCursor_ClassDecl, "class"},
  {CXCursor_StructDecl, "class"},
  {CXCursor_CXXMethod, "function"},
  {CXCursor_Constructor, "function"},
  {CXCursor_FieldDecl, "field"},
  {CXCursor_FunctionDecl, "function"},
  {CXCursor_FunctionTemplate, "function"},
};

std::string nameOf(CXCursorKind kind)
{
  const auto itr = KindName.find(kind);
  if (itr == KindName.end()) {
    return "<" + std::to_string(kind) + ">";
  } else {
    return itr->second;
  }
}

bool ignore(CXCursor cursor)
{
  const auto location = clang_getCursorLocation(cursor);
  return clang_Location_isInSystemHeader(location);
}

CXChildVisitResult Clast::visit_children(CXCursor cursor, CXCursor parent, CXClientData data)
{
  if (ignore(cursor)) {
    return CXChildVisit_Continue;
  }

  VisitorData* vd = static_cast<VisitorData*>(data);

  const auto kind = clang_getCursorKind(cursor);

  switch (kind) {
    case CXCursor_Namespace:
      {
        const auto def = clang_getCursorDefinition(cursor);
        const auto usr = Clang::getCursorUSR(def);
        const auto itr = vd->visited->find(usr);
        const bool visited = (itr != vd->visited->end());

        Node* node;

        if (visited) {
          node = itr->second;
        } else {
          std::string kname = nameOf(kind);
          std::string name = Clang::getCursorSpelling(cursor);
          node = new Node(name, kname);
          (*vd->visited)[usr] = node;
          vd->parent->children.push_back(node);
        }

        VisitorData nvd{node, vd->visited};
        clang_visitChildren(cursor, visit_children, &nvd);
        break;
      }

    case CXCursor_UnexposedDecl:
    case CXCursor_ClassDecl:
    case CXCursor_TypedefDecl:
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
    case CXCursor_EnumDecl:
    case CXCursor_TypeAliasDecl:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_FieldDecl:
    case CXCursor_FunctionDecl:
    case CXCursor_FunctionTemplate:
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
    {
        //TODO make it work with multiple input files

        const auto def = clang_getCursorDefinition(cursor); // in order to traverse the method bodies
        const auto usr = Clang::getCursorUSR(def);
        const bool visited = (vd->visited->find(usr) != vd->visited->end());

        if (!visited) {
          std::string kname = nameOf(kind);
          std::string name = Clang::getCursorSpelling(cursor);
          Node* node = new Node(name, kname);
          const auto loc = utils::location(def);
          node->file = loc.first;
          node->line = loc.second;
          vd->parent->children.push_back(node);
          (*vd->visited)[usr] = node;
          VisitorData nvd{node, vd->visited};
          clang_visitChildren(def, visit_children, &nvd);
        }

        break;
      }

    case CXCursor_TypeRef:
    case CXCursor_CXXBaseSpecifier:
    case CXCursor_TemplateRef:
//    case CXCursor_NamespaceRef:
    case CXCursor_MemberRef:
    case CXCursor_LabelRef:
    case CXCursor_OverloadedDeclRef:
    case CXCursor_VariableRef:
    case CXCursor_DeclRefExpr:
    case CXCursor_MemberRefExpr:
    case CXCursor_CallExpr:
    {
      const CXCursor referenced = clang_getCursorReferenced(cursor);

      if (!ignore(referenced)) {
        const auto usr = Clang::getCursorUSR(referenced);
        vd->parent->references.insert(usr);
      }
      clang_visitChildren(cursor, visit_children, vd);
      break;
    }

    default:
      clang_visitChildren(cursor, visit_children, vd);
      break;
  }

  return CXChildVisit_Continue;
}

VisitorData *Clast::getData()
{
  return &data;
}


void Clast::write(const Node* node, XmlWriter& writer) const
{
  writer.startNode(node->type);
  writer.attribute("name", node->name);
  writer.attribute("id", std::to_string(reinterpret_cast<long>(node)));
  if (node->file != "") {
    writer.attribute("file", node->file);
  }
  if (node->line != 0) {
    writer.attribute("line", std::to_string(node->line));
  }

  for (const auto& ref : node->references) {
    const auto itr = visited.find(ref);
    if (itr != visited.end()) {
      writer.startNode("reference");
      writer.attribute("target", std::to_string(reinterpret_cast<long>(itr->second)));
      writer.endNode();
    }
  }

  for (const auto& child : node->children) {
    write(child, writer);
  }

  writer.endNode();
}

void Clast::report(std::ostream & os) const
{
  XmlWriter writer{os};
  write(&root, writer);
}

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