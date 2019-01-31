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

CXChildVisitResult Dependency::collect_base_classes(
    CXCursor cursor,
    CXCursor parent,
    CXClientData data)
{
  std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

  const auto kind = clang_getCursorKind(cursor);
  switch (kind) {
    case CXCursor_CXXBaseSpecifier:
      base_classes->push_back(clang_getCursorDefinition(cursor));
      break;

    case CXCursor_FieldDecl:
//      clang_visitChildren(cursor, collect_base_classes, data);
      break;

    case CXCursor_TypeRef:
      //TODO also check as method arguments?
      //TODO also check when only used within a method?
#warning only add when type is a class (our class?)
      base_classes->push_back(clang_getCursorDefinition(cursor));	//TODO only add when type is a class
      break;

//    case CXCursor_Namespace:
//    case CXCursor_NamespaceAlias:
//    case CXCursor_NamespaceRef:
//      {
//        const auto def = clang_getCursorDefinition(cursor);
//        std::cout << namespace_for(def) << Clang::getCursorSpelling(def) << std::endl;
//        break;
//      }

    default:
      //			std::cout << clang_getCursorKind(cursor) << std::endl;
      break;
  }

  clang_visitChildren(cursor, collect_base_classes, data);

  return CXChildVisit_Continue;
}

void Dependency::collect_base_classes(CXCursor cursor)
{
  std::vector<CXCursor> bases;

  const auto child = path_for(cursor);
  graph[child];

  namespaces[{child.begin(), child.end()-1}].insert(child.back());

  clang_visitChildren(cursor, collect_base_classes, &bases);

  for (auto base : bases) {
    const auto parent = path_for(base);
    graph[child].insert(parent);
  }
}

CXChildVisitResult Dependency::visit(
    CXCursor cursor,
    CXCursor parent)
{
  if (ignore(cursor))
    return CXChildVisit_Continue;
  if (Clang::getCursorKind(cursor) != CXCursor_ClassDecl)
    return CXChildVisit_Recurse;

  std::string usr = Clang::getCursorUSR(cursor);

  std::ostringstream os;
  collect_base_classes(cursor);

  return CXChildVisit_Recurse;
}

void Dependency::report(std::ostream & os) const
{
  using namespace std;

  os << get_name() << endl;

  os << "digraph" << std::endl;
  os << "{" << std::endl;
  os << "rankdir=\"BT\";" << std::endl;
  os << "node[shape = box];" << std::endl;
  os << std::endl;

  unsigned number = 0;
  for (const auto& itr : namespaces) {
    const std::string name = print(itr.first);
    os << "subgraph cluster_" << number << " {" << std::endl;
    number++;
    os << "label = \"" << name << "\";" << std::endl;
    for (const auto& clas : itr.second) {
      const std::string cn = name + "::" + clas;
      os << escape(cn) << " [label=\"" + clas + "\"]" <<  std::endl;
    }
    os << "}" << std::endl;
  }


//  for (const auto& itr : graph) {
//    const std::string name = print(itr.first);
//    os << escape(name) << " [label=\"" + name + "\"]" <<  std::endl;
//  }

  os << std::endl;

  for (const auto& itr : graph) {
    for (const auto& dest : itr.second) {
      // only print dependency when it is to a class we defined in our project
      //FIXME is this ok?
      const auto idx = graph.find(dest);
      if (idx != graph.end()) {
        const std::string source = print(itr.first);
        const std::string destination = print(dest);
        os << escape(source) << " -> " << escape(destination) << std::endl;
      }
    }
  }

  os << "}" << std::endl;
}

void Dependency::collect(ResultContainer & container) const
{
}

