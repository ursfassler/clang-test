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

<<<<<<< HEAD:metrics/Metric_IT.hpp
		std::map<std::string, std::set<std::string>> graph{};
		typedef std::string Member;
		typedef std::string Method;
		std::map<Method, std::set<Member>> kohesion;

		void collect_member_references(CXCursor cursor);
		static CXChildVisitResult collect_member_references(CXCursor cursor, CXCursor parent, CXClientData data);
		void reportKohesion(std::ostream &os) const;
		void reportDependencies(std::ostream &os) const;
		std::string formatMember(CXCursor cursor);
public:
=======
    typedef std::vector<std::string> Path;
    std::map<Path, std::set<Path>> graph{};
    std::map<Path, std::set<std::string>> namespaces{};

	public:
>>>>>>> wip: rename to dependency:metrics/dependency.hpp
		virtual const std::string & get_name() const;
		virtual const std::string & get_id() const;

		virtual CXChildVisitResult visit(
				CXCursor cursor,
				CXCursor parent);

		virtual void report(std::ostream &) const;
		virtual void collect(ResultContainer &) const;

		static void register_in(VisitorFactory &);
};
