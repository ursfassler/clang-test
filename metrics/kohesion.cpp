#include "kohesion.hpp"
#include "Clang.hpp"
#include "utils.hpp"
#include <vector>
#include <sstream>

namespace metric
{


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

  clang_visitChildren(cursor, collect_member_references, &members);

  for (auto member : members) {
    const auto prettyMember = utils::getPath(member);
    bgraph.addEdge(method, prettyMember);
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
//  graphviz::Writer graphviz{os};
//  graph.writeTo(graphviz);
  bgraph.serialize(os);
}

std::string Kohesion::name() const
{
  return "kohesion";
}

const graphviz::Graph &Kohesion::graph() const
{
  return bgraph;
}


}
