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
		case CXCursor_MemberRefExpr:
			base_classes->push_back(cursor);
			return CXChildVisit_Recurse;

		case CXCursor_CXXMethod:
		case CXCursor_FunctionDecl:
		case CXCursor_CallExpr:
			return CXChildVisit_Continue;

		default:
//			std::cout << clang_getCursorKind(cursor) << std::endl;
			return CXChildVisit_Recurse;
		}


	return CXChildVisit_Continue;
}


void Metric_IT::collect_member_references(CXCursor cursor)
{
	std::vector<CXCursor> bases;

	const auto method = namespace_for(cursor) + Clang::getCursorSpelling(cursor);
	kohesion[method];

	clang_visitChildren(cursor, collect_member_references, &bases);

	for (auto base : bases) {
			const auto pretty = namespace_for(base) + Clang::getCursorSpelling(base);
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

std::string escape(std::string value)
{
	std::string result;

	result += "_";

	for (const auto& sym : value) {
			switch (sym) {
				case '_':
					result += "_5f";
					break;
				case ':':
					result += "_3a";
					break;
				case '=':
					result += "_3d";
					break;
				case '[':
					result += "_5b";
					break;
				case ']':
					result += "_5d";
					break;
				case '<':
					result += "_3c";
					break;
				case '>':
					result += "_3e";
					break;
				case '!':
					result += "_21";
					break;
				case '*':
					result += "_2a";
					break;
				case '+':
					result += "_2b";
					break;
				case '-':
					result += "_2d";
					break;

				default:
					result += sym;
				}
		}

	return result;
}

void Metric_IT::report(std::ostream & os) const
{
	using namespace std;

//	const auto& data = graph;
	const auto& data = kohesion;

	os << get_name() << endl;

	os << "digraph" << std::endl;
	os << "{" << std::endl;
	os << "rankdir=\"LR\";" << std::endl;
	os << "node[shape = box];" << std::endl;
	os << std::endl;

	for (const auto& itr : data) {
			os << escape(itr.first) << " [label=\"" + itr.first + "\"]" <<  std::endl;
		}

	os << std::endl;

	for (const auto& itr : data) {
			for (const auto& dest : itr.second) {
					// only print dependency when it is to a class we defined in our project
					//FIXME is this ok?
					const auto idx = data.find(dest);
//					if (idx != data.end()) {
							os << escape(itr.first) << " -> " << escape(dest) << std::endl;
//						}
				}
		}

	os << "}" << std::endl;
}

void Metric_IT::collect(ResultContainer & container) const
{
}

