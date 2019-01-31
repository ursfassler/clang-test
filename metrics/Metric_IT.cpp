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

CXChildVisitResult Metric_IT::collect_base_classes(
		CXCursor cursor,
		CXCursor parent,
		CXClientData data)
{
	std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

	const auto kind = clang_getCursorKind(cursor);
	switch (kind) {
		case CXCursor_CXXBaseSpecifier:
//			base_classes->push_back(clang_getCursorDefinition(cursor));
			break;

		case CXCursor_FieldDecl:
//			clang_visitChildren(cursor, collect_base_classes, data);
			break;

		case CXCursor_TypeRef:
			//TODO also check as method arguments?
			//TODO also check when only used within a method?
#warning only add when type is a class (our class?)
			base_classes->push_back(clang_getCursorDefinition(cursor));	//TODO only add when type is a class
			break;

		default:
//			clang_visitChildren(cursor, collect_base_classes, data);
//			std::cout << clang_getCursorKind(cursor) << std::endl;
			break;
		}

	clang_visitChildren(cursor, collect_base_classes, data);

	return CXChildVisit_Continue;
}

void Metric_IT::collect_base_classes(CXCursor cursor)
{
	std::vector<CXCursor> bases;

	const auto child = namespace_for(cursor) + Clang::getCursorSpelling(cursor);
	graph[child];

	clang_visitChildren(cursor, collect_base_classes, &bases);

	for (auto base : bases) {
			const auto parent = namespace_for(base) + Clang::getCursorSpelling(base);
			graph[child].insert(parent);
	}
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

//	if (Clang::getCursorKind(cursor) == CXCursor_ClassDecl) {
//		collect_base_classes(cursor);
//		}


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

void Metric_IT::reportDependencies(std::ostream & os) const
{
	using namespace std;

	const auto& data = graph;

	os << "digraph" << std::endl;
	os << "{" << std::endl;
	os << "rankdir=\"TB\";" << std::endl;
	os << "node[shape = box];" << std::endl;
	os << std::endl;

	for (const auto& itr : data) {
			if (!itr.second.empty()) {
					os << escape(itr.first) << " [label=\"" + itr.first + "\"]" <<  std::endl;
				}
		}

	os << std::endl;

	for (const auto& itr : data) {
			for (const auto& dest : itr.second) {
					// only print dependency when it is to a class we defined in our project
					//FIXME is this ok?
					const auto idx = data.find(dest);
					if (idx != data.end()) {
							os << escape(itr.first) << " -> " << escape(dest) << std::endl;
						}
				}
		}

	os << "}" << std::endl;
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

