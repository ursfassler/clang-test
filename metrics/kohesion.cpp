#include "kohesion.hpp"
#include "Clang.hpp"
#include "graphviz.hpp"
#include "utils.hpp"
#include <vector>

CXChildVisitResult Kohesion::collect_member_references(CXCursor cursor, CXCursor, CXClientData data)
{
  std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

  const auto kind = clang_getCursorKind(cursor);
  switch (kind) {
    case CXCursor_MemberRef:
    case CXCursor_MemberRefExpr: {
        const CXCursor referenced = clang_getCursorReferenced(cursor);
        const auto refkind = clang_getCursorKind(referenced);

        // needed to filter out access to methods
        if (refkind == CXCursor_FieldDecl) {
          base_classes->push_back(referenced);
        }

        break;
      }

    default:
      break;
  }


  return CXChildVisit_Recurse;
}

void Kohesion::collect_member_references(CXCursor cursor)
{
  std::vector<CXCursor> members;

  const auto method = utils::getPath(cursor);
  graph[method];

  clang_visitChildren(cursor, collect_member_references, &members);

  for (auto member : members) {
    const auto prettyMember = utils::getPath(member);
    graph[method].insert(prettyMember);
  }
}

CXChildVisitResult Kohesion::visit(CXCursor cursor, CXCursor)
{
  if (ignore(cursor))
    return CXChildVisit_Continue;

  if (Clang::getCursorKind(cursor) == CXCursor_CXXMethod) {
    collect_member_references(cursor);
    return CXChildVisit_Continue;
  }

  return CXChildVisit_Recurse;
}

void Kohesion::report(std::ostream & os) const
{
  reportKohesion(os);
}

void Kohesion::reportKohesion(std::ostream & os) const
{
  graphviz::Graph g{};

  for (const auto& itr : graph) {
    if (!itr.second.empty()) {
      g.addNode(itr.first);
    }
  }

  for (const auto& itr : graph) {
    for (const auto& dest : itr.second) {
      g.addEdge(itr.first, dest);
    }
  }

  graphviz::Writer graphviz{os};
  g.writeTo(graphviz);
}
