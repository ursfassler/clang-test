#pragma once

#include "Visitor.hpp"
#include "VisitorDescriptor.hpp"
#include <map>
#include <set>
#include <vector>

class VisitorFactory;

class Dependency :
		public Visitor
{
	private:
		static const VisitorDescriptor DESCRIPTOR;

		void collect_base_classes(CXCursor cursor);

		static CXChildVisitResult collect_base_classes(
				CXCursor cursor,
				CXCursor parent,
				CXClientData data);

    typedef std::vector<std::string> Path;
    std::map<Path, std::set<Path>> graph{};

	public:
		virtual const std::string & get_name() const;
		virtual const std::string & get_id() const;

		virtual CXChildVisitResult visit(
				CXCursor cursor,
				CXCursor parent);

		virtual void report(std::ostream &) const;
		virtual void collect(ResultContainer &) const;

		static void register_in(VisitorFactory &);
};
