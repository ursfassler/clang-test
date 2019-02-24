#include "clast.hpp"
#include "Clang.hpp"
#include "utils.hpp"
#include "XmlWriter.hpp"
#include <iostream>


namespace metric
{


std::map<CXCursorKind, std::string> KindName
{
  {CXCursor_Namespace, "package"},
  {CXCursor_TypedefDecl, "class"},
  {CXCursor_ClassDecl, "class"},
  {CXCursor_StructDecl, "class"},
  {CXCursor_EnumDecl, "class"},
  {CXCursor_UnionDecl, "class"},
  {CXCursor_ClassTemplate, "class"},
  {CXCursor_TypeAliasDecl, "class"},
  {CXCursor_CXXMethod, "method"},
  {CXCursor_Constructor, "method"},
  {CXCursor_Destructor, "method"},
  {CXCursor_FieldDecl, "field"},
  {CXCursor_FunctionDecl, "function"},
  {CXCursor_FunctionTemplate, "function"},
};

std::string nameOf(CXCursorKind kind)
{
  const auto itr = KindName.find(kind);
  if (itr == KindName.end()) {
    std::cerr << "unknown cursor kind: " << kind << std::endl;
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

bool isAnonymousNamespace(CXCursor cursor)
{
  const auto kind = clang_getCursorKind(cursor);

  if (kind == CXCursor_Namespace) {
    std::string name = Clang::getCursorSpelling(cursor);
    return name == "";
  } else {
    return false;
  }
}

Node* findParent(CXCursor cursor, VisitorData* vd)
{
  auto sparent = clang_getCursorSemanticParent(cursor);

  while (isAnonymousNamespace(sparent)) {
    sparent = clang_getCursorSemanticParent(sparent);
  }

  if (clang_getCursorKind(sparent) == CXCursor_TranslationUnit) {
    if (vd->parent != vd->root) {
      std::cerr << "decleration missed" << std::endl;
    }
    return vd->root;
  } else {
    const auto parentIdx = vd->visited->find(Clang::getCursorUSR(sparent));
    if (parentIdx == vd->visited->end()) {
      return nullptr;
    } else {
      return parentIdx->second;
    }
  }
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

        if (isAnonymousNamespace(cursor)) {
          // we merge anonymous namespaces into their parent namespace
          clang_visitChildren(cursor, visit_children, vd);
        } else {
          Node* node;

          if (visited) {
            node = itr->second;
          } else {
            const std::string name = Clang::getCursorSpelling(cursor);
            const std::string kname = nameOf(kind);
            node = new Node(name, kname);
            (*vd->visited)[usr] = node;

            Node* pnode = findParent(cursor, vd);
            if (pnode) {
              pnode->children.push_back(node);
            } else {
              std::cerr << "parent not found for: " << name << std::endl;
            }
          }

          VisitorData nvd{node, vd->root, vd->visited};
          clang_visitChildren(cursor, visit_children, &nvd);
        }

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
        std::string print = Clang::getCursorSpelling(cursor);

        const auto def = clang_getCursorDefinition(cursor); // in order to traverse the method bodies
        const auto valid = !clang_isInvalid(clang_getCursorKind(def));
        if (valid) {
          const auto usr = Clang::getCursorUSR(def);
          const bool visited = (vd->visited->find(usr) != vd->visited->end());

          if (!visited) {
            std::string kname = nameOf(kind);
            std::string name = Clang::getCursorSpelling(cursor);

            Node* node = new Node(name, kname);
            const auto loc = utils::location(def);
            node->file = loc.first;
            node->line = loc.second;
            (*vd->visited)[usr] = node;

            Node* pnode = findParent(cursor, vd);
            if (pnode) {
              pnode->children.push_back(node);
            } else {
              std::cerr << "parent not found for: " << name << std::endl;
            }

            VisitorData nvd{node, vd->root, vd->visited};
            clang_visitChildren(def, visit_children, &nvd);
          }
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
        const auto parentKind = clang_getCursorKind(parent);
        switch (parentKind) {
          case CXCursor_CXXMethod:
          case CXCursor_Constructor:
          case CXCursor_Destructor:
            // don't add reference to defining parent
            break;

          default: {
              const auto usr = Clang::getCursorUSR(referenced);
              vd->parent->references.insert(usr);
              break;
            }
        }
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
  writer.attribute("id", idOf(node));
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
      writer.attribute("target", idOf(itr->second));
      writer.endNode();
    }
  }

  for (const auto& child : node->children) {
    write(child, writer);
  }

  writer.endNode();
}

std::string Clast::idOf(const Node* node) const
{
  return std::to_string(reinterpret_cast<long>(node));
}

void Clast::report(XmlWriter& writer) const
{
  write(&root, writer);
}


}
