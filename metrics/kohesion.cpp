#include "kohesion.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include "graphviz.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

const VisitorDescriptor Kohesion::DESCRIPTOR =
{
  "Ko",
  "Kohesion",
	"",
	true,
	true,
	false,
};

void Kohesion::register_in(VisitorFactory & factory)
{
	factory.add(DESCRIPTOR, []()
	{
    return new Kohesion;
	});
}

const std::string & Kohesion::get_name() const
{
	return DESCRIPTOR.name;
}

const std::string & Kohesion::get_id() const
{
	return DESCRIPTOR.id;
}

CXChildVisitResult Kohesion::collect_member_references(
		CXCursor cursor,
		CXCursor parent,
		CXClientData data)
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
			return CXChildVisit_Recurse;
			}

		default:
//			std::cout << clang_getCursorKind(cursor) << std::endl;
			return CXChildVisit_Recurse;
		}


	return CXChildVisit_Continue;
}

Kohesion::Path Kohesion::getPath(CXCursor cursor)
{
  Path result{};

  for (;;) {
    result.push_back(Clang::getCursorSpelling(cursor));

    cursor = clang_getCursorSemanticParent(cursor);
    CXCursorKind kind = Clang::getCursorKind(cursor);

    if (kind == CXCursor_TranslationUnit) {
      break;
    }
  }

  std::reverse(result.begin(), result.end());

  return result;
}

void Kohesion::collect_member_references(CXCursor cursor)
{
  std::vector<CXCursor> members;

  const auto method = getPath(cursor);
  graph[method];

  clang_visitChildren(cursor, collect_member_references, &members);

  for (auto member : members) {
      const auto prettyMember = getPath(member);
      graph[method].insert(prettyMember);
  }
}

CXChildVisitResult Kohesion::visit(
		CXCursor cursor,
		CXCursor parent)
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

void Kohesion::collect(ResultContainer & container) const
{
}