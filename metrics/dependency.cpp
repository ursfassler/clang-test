#include "dependency.hpp"
#include "Clang.hpp"
#include "VisitorFactory.hpp"
#include "utils.hpp"
#include <vector>


namespace metric
{


bool Dependency::isInProject(CXCursor value) const
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

CXChildVisitResult Dependency::collect_references(CXCursor cursor, CXCursor, CXClientData data)
{
  std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

  const auto kind = clang_getCursorKind(cursor);
  switch (kind) {
    case CXCursor_TypeRef:
      base_classes->push_back(cursor);
      break;

    default:
      break;
  }

  return CXChildVisit_Recurse;
}

const graphviz::Graph &Dependency::graph() const
{
  return bgraph;
}

void Dependency::collect_references(CXCursor clazz, CXCursor root)
{
  std::vector<CXCursor> bases;

  if (!isInProject(clazz)) {
    return;
  }

  const auto child = utils::getPath(clazz);
  bgraph.addNode(child);

  clang_visitChildren(root, collect_references, &bases);

  for (auto base : bases) {
    const auto defintion = clang_getCursorDefinition(base);
    if (isInProject(defintion)) {
      const auto location = utils::location(base);
      const std::string name = location.first + ":" + std::to_string(location.second);
      const auto parent = utils::getPath(defintion);
      bgraph.addEdge(child, parent, name);
    }
  }
}

CXChildVisitResult Dependency::visit(CXCursor cursor, CXCursor)
{
  if (ignore(cursor)) {
    return CXChildVisit_Continue;
  }

  const auto kind = Clang::getCursorKind(cursor);
  switch (kind) {
    case CXCursor_ClassDecl:
    case CXCursor_TypedefDecl:
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
    case CXCursor_EnumDecl:
    case CXCursor_TypeAliasDecl:
      collect_references(cursor, cursor);
      break;

    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_FieldDecl:
      {
        const auto clazz = clang_getCursorSemanticParent(cursor);
        collect_references(clazz, cursor);
        break;
      }

    default:
      break;
  }

  return CXChildVisit_Recurse;
}

std::string Dependency::name() const
{
  return "dependency";
}

void Dependency::report(std::ostream & os) const
{
//  graphviz::Writer writer{os};
//  graph.writeTo(writer);
  bgraph.serialize(os);
}


}
