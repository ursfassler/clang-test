#include "dependency.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include "graphviz.hpp"
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

CXChildVisitResult Dependency::collect_references(CXCursor cursor, CXCursor parent, CXClientData data)
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

void Dependency::collect_references(CXCursor cursor)
{
  std::vector<CXCursor> bases;

  const auto child = path_for(cursor);
  graph[child];

  clang_visitChildren(cursor, collect_references, &bases);

  for (auto base : bases) {
    const auto parent = path_for(base);
    graph[child].insert(parent);
  }
}

CXChildVisitResult Dependency::visit(
    CXCursor cursor,
    CXCursor parent)
{
  if (ignore(cursor)) {
    return CXChildVisit_Continue;
  }

  if (Clang::getCursorKind(cursor) == CXCursor_ClassDecl) {
    collect_references(cursor);
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

void Dependency::collect(ResultContainer & container) const
{
}

