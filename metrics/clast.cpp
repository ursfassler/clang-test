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
          node->file = usr;
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

void write(const Node* node, unsigned level, std::ostream& os, const std::map<std::string, Node*>& nodes)
{
  os << std::string(level * 4, ' ');
  os << node->name << "(" << node->type << ")";
  os << " " << node->file << ":" << node->line;

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



}
