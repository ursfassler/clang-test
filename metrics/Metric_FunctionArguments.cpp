#include "Metric_FunctionArguments.hpp"
#include "Clang.hpp"
#include "Location.hpp"
#include "VisitorFactory.hpp"
#include <iomanip>
#include <sstream>

const VisitorDescriptor Metric_FunctionArguments::DESCRIPTOR =
{
	"NumFuncArg",
	"Number of Funtion/Method Arguments",
	"",
	true,
	false,
	true,
};

void Metric_FunctionArguments::register_in(VisitorFactory & factory)
{
	factory.add(DESCRIPTOR, []()
	{
		return new Metric_FunctionArguments;
	});
}

const std::string & Metric_FunctionArguments::get_name() const
{
	return DESCRIPTOR.name;
}

const std::string & Metric_FunctionArguments::get_id() const
{
	return DESCRIPTOR.id;
}

CXChildVisitResult Metric_FunctionArguments::visit(
		CXCursor cursor,
		CXCursor parent)
{
	if (ignore(cursor))
		return CXChildVisit_Continue;

	switch (Clang::getCursorKind(cursor)) {
		case CXCursor_CXXMethod:
		case CXCursor_FunctionDecl:
			break;

		default:
			return CXChildVisit_Recurse;
	}

	std::string usr = Clang::getCursorUSR(cursor);
	if (data.find(usr) != data.end())
		return CXChildVisit_Recurse;

	std::ostringstream os;
	os << Clang::getNumArguments(cursor);

	data[usr] = { cursor, os.str() };

	return CXChildVisit_Recurse;
}

void Metric_FunctionArguments::report(std::ostream & os) const
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

void Metric_FunctionArguments::collect(ResultContainer & container) const
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

