#include "dependency.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include "graphviz.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

const VisitorDescriptor Dependency::DESCRIPTOR =
{
  "Dep",
  "Dependencies",
  "",
  true,
  true,
  false,
};

void Dependency::register_in(VisitorFactory & factory)
{
  factory.add(DESCRIPTOR, []()
  {
    return new Dependency;
  });
}

const std::string & Dependency::get_name() const
{
  return DESCRIPTOR.name;
}

const std::string & Dependency::get_id() const
{
  return DESCRIPTOR.id;
}

CXChildVisitResult Dependency::collect_references(CXCursor cursor, CXCursor, CXClientData data)
{
  std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

  const auto kind = clang_getCursorKind(cursor);
  switch (kind) {
    case CXCursor_TypeRef:
      base_classes->push_back(clang_getCursorDefinition(cursor));
      break;

    default:
      break;
  }

  return CXChildVisit_Recurse;
}

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

void Dependency::collect_references(CXCursor clazz, CXCursor root)
{
  std::vector<CXCursor> bases;

  if (!isInProject(clazz)) {
    return;
  }

  const auto child = utils::getPath(clazz);
  graph[child];

  clang_visitChildren(root, collect_references, &bases);

  for (auto base : bases) {
    if (isInProject(base)) {
      const auto parent = utils::getPath(base);
      graph[child].insert(parent);
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

void Dependency::report(std::ostream & os) const
{
  graphviz::Graph g{};

  for (const auto& itr : graph) {
    g.addNode(itr.first);
    for (const auto& dest : itr.second) {
      g.addEdge(itr.first, dest);
    }
  }

  graphviz::Writer writer{os};
  g.writeTo(writer);
}

void Dependency::collect(ResultContainer&) const
{
}

