#include "Metric_IT.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include "graphviz.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

const VisitorDescriptor Metric_IT::DESCRIPTOR =
{
	"IT",
	"Inheritance Tree",
	"",
	true,
	true,
	false,
};

void Metric_IT::register_in(VisitorFactory & factory)
{
	factory.add(DESCRIPTOR, []()
	{
		return new Metric_IT;
	});
}

const std::string & Metric_IT::get_name() const
{
	return DESCRIPTOR.name;
}

const std::string & Metric_IT::get_id() const
{
	return DESCRIPTOR.id;
}

CXChildVisitResult Metric_IT::collect_member_references(
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

std::string Metric_IT::formatMember(CXCursor cursor)
{
	return namespace_for(cursor) + Clang::getCursorSpelling(clang_getCursorSemanticParent(cursor)) + "." + Clang::getCursorSpelling(cursor);
}

void Metric_IT::collect_member_references(CXCursor cursor)
{
	std::vector<CXCursor> bases;

	const auto method = formatMember(cursor);
	kohesion[method];

	clang_visitChildren(cursor, collect_member_references, &bases);

	for (auto base : bases) {
			const auto pretty = formatMember(base);
			kohesion[method].insert(pretty);
	}
}

CXChildVisitResult Metric_IT::visit(
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

void Metric_IT::report(std::ostream & os) const
{
  reportKohesion(os);
}

void Metric_IT::reportKohesion(std::ostream & os) const
{
	using namespace std;


	os << "digraph" << std::endl;
	os << "{" << std::endl;
	os << "rankdir=\"LR\";" << std::endl;
	os << "node[shape = box];" << std::endl;
	os << std::endl;

	std::set<std::string> fields{};
	for (const auto& itr : kohesion) {
			if (!itr.second.empty()) {
					os << escape(itr.first) << " [label=\"" + itr.first + "\"]" <<  std::endl;
				}
			fields.insert(itr.second.cbegin(), itr.second.cend());
		}
	os << std::endl;

	for (const auto& itr : fields) {
			os << escape(itr) << " [label=\"" + itr + "\"]" <<  std::endl;
		}
	os << std::endl;

	for (const auto& itr : kohesion) {
			for (const auto& dest : itr.second) {
					os << escape(itr.first) << " -> " << escape(dest) << std::endl;
				}
		}

	os << "}" << std::endl;
}

void Metric_IT::collect(ResultContainer & container) const
{
}

