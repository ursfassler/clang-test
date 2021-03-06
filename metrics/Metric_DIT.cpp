#include "Metric_DIT.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

const VisitorDescriptor Metric_DIT::DESCRIPTOR =
{
	"DIT",
	"Depth of Inheritance Tree",
	"",
	true,
	true,
	false,
};

void Metric_DIT::register_in(VisitorFactory & factory)
{
	factory.add(DESCRIPTOR, []()
	{
		return new Metric_DIT;
	});
}

const std::string & Metric_DIT::get_name() const
{
	return DESCRIPTOR.name;
}

const std::string & Metric_DIT::get_id() const
{
	return DESCRIPTOR.id;
}

CXChildVisitResult Metric_DIT::collect_base_classes(
		CXCursor cursor,
		CXCursor parent,
		CXClientData data)
{
	std::vector<CXCursor> * base_classes = static_cast<std::vector<CXCursor> *>(data);

	if (clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier)
		base_classes->push_back(clang_getCursorDefinition(cursor));

	return CXChildVisit_Continue;
}

unsigned int Metric_DIT::count_depth_of_inheritance_tree(CXCursor cursor) const
{
	std::vector<CXCursor> bases;
	unsigned int dit_max = 0;

	clang_visitChildren(cursor, collect_base_classes, &bases);

	for (auto base : bases) {
		unsigned int t = 1 + count_depth_of_inheritance_tree(base);
		if (t > dit_max)
			dit_max = t;
	}

	return dit_max;
}

CXChildVisitResult Metric_DIT::visit(
		CXCursor cursor,
		CXCursor parent)
{
	if (ignore(cursor))
		return CXChildVisit_Continue;
	if (Clang::getCursorKind(cursor) != CXCursor_ClassDecl)
		return CXChildVisit_Recurse;

	std::string usr = Clang::getCursorUSR(cursor);
	if (data.find(usr) != data.end())
		return CXChildVisit_Recurse;

	std::ostringstream os;
	os << count_depth_of_inheritance_tree(cursor);

	data[usr] = { cursor, os.str() };

	return CXChildVisit_Recurse;
}

void Metric_DIT::report(std::ostream & os) const
{
	using namespace std;

	os << get_name() << endl;

	for (auto i : data) {
		os
			<< setw(3)
			<< i.second.result
			<< " "
			<< namespace_for(i.second.cursor) << Clang::getCursorSpelling(i.second.cursor)
			<< " (" << Location(i.second.cursor) << ")"
			<< endl;
	}
}

void Metric_DIT::collect(ResultContainer & container) const
{
	for (auto i : data) {
		container.insert(ResultContainer::value_type(
			i.first,
			{
				i.second.cursor,
				get_id(),
				i.second.result,
			}));
	}
}

