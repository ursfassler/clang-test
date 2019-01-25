#include "Metric_IT.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
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

	if (clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier)
		base_classes->push_back(clang_getCursorDefinition(cursor));

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

CXChildVisitResult Metric_IT::visit(
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

std::string escape(std::string value)
{
	std::replace(value.begin(), value.end(), ':', '_');
	return value;
}

void Metric_IT::report(std::ostream & os) const
{
	using namespace std;

	os << get_name() << endl;

	os << "digraph" << std::endl;
	os << "{" << std::endl;
	os << "rankdir=\"BT\";" << std::endl;
	os << "node[shape = box];" << std::endl;
	os << std::endl;

	for (const auto& itr : graph) {
			os << escape(itr.first) << " [label=\"" + itr.first + "\"]" <<  std::endl;
		}

	os << std::endl;

	for (const auto& itr : graph) {
			for (const auto& dest : itr.second) {
					os << escape(itr.first) << " -> " << escape(dest) << std::endl;
				}
		}

	os << "}" << std::endl;
}

void Metric_IT::collect(ResultContainer & container) const
{
}

