#include "clast.hpp"
#include "Clang.hpp"
#include "VisitorFactory.hpp"
#include "utils.hpp"
#include <vector>


#include <iostream>


namespace metric
{


bool Clast::isInProject(CXCursor value)
{
  const auto sl = clang_getCursorLocation(value);
  CXFile file;
  clang_getFileLocation(sl, &file, nullptr, nullptr, nullptr);
  const auto cxfilename = clang_getFileName(file);
  const std::string filename = Clang::to_string(cxfilename);

  const std::string projectPath = "/home/"; //TODO find better way
  const auto prefix = filename.substr(0, projectPath.size());
  const auto isInProject = prefix == projectPath;

  return isInProject;
}

std::map<CXCursorKind, std::string> KindName
{
  {CXCursor_Namespace, "namespace"},
  {CXCursor_TypedefDecl, "typedef"},
  {CXCursor_ClassDecl, "class"},
  {CXCursor_StructDecl, "struct"},
  {CXCursor_CXXMethod, "method"},
  {CXCursor_Constructor, "constructor"},
  {CXCursor_FieldDecl, "field"},
  {CXCursor_FunctionDecl, "function"},
  {CXCursor_FunctionTemplate, "function template"},
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

CXChildVisitResult Clast::visit_children(CXCursor cursor, CXCursor, CXClientData data)
{
  if (ignore(cursor)) {
    return CXChildVisit_Continue;
  }

  VisitorData* vd = static_cast<VisitorData*>(data);

  const auto kind = clang_getCursorKind(cursor);

  switch (kind) {
    case CXCursor_Namespace:
      {
      std::string kname = nameOf(kind);
      std::string name = Clang::getCursorSpelling(cursor);
      Node* node = new Node(name + "(" + kname + ")");
      vd->parent->children.push_back(node);
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
        const auto def = clang_getCursorDefinition(cursor); // in order to traverse the method bodies
        const auto usr = Clang::getCursorUSR(def);
        const bool visited = (vd->visited->find(usr) != vd->visited->end());

        if (!visited) {
          std::string kname = nameOf(kind);
          std::string name = Clang::getCursorSpelling(cursor);
          Node* node = new Node(name + "(" + kname + ")");
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

CXChildVisitResult Clast::visit(CXCursor cursor, CXCursor parent)
{
  VisitorData data{&this->root, &visited};

  return visit_children(cursor, parent,  &data);
}

std::string Clast::name() const
{
  return "clast";
}

void write(const Node* node, unsigned level, std::ostream& os, const std::map<std::string, Node*>& nodes)
{
  os << std::string(level * 4, ' ');
  os << node->name;

  for (const auto& ref : node->references) {
    const auto itr = nodes.find(ref);
    if (itr != nodes.end()) {
      os << " " << itr->second->name;
    }
  }
  os << std::endl;

  for (const auto& child : node->children) {
    write(child, level+1, os, nodes);
  }
}

void Clast::report(std::ostream & os) const
{
  write(&root, 0, os, visited);
}

const graphviz::Graph &Clast::graph() const
{
  static graphviz::Graph g{};
  return g;
}



}
