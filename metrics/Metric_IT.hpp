#ifndef __METRIC_DIT__HPP__
#define __METRIC_DIT__HPP__

#include "Visitor.hpp"
#include "VisitorDescriptor.hpp"
#include <map>
#include <set>
#include <vector>

class VisitorFactory;

class Metric_IT :
		public Visitor
{
	private:
		static const VisitorDescriptor DESCRIPTOR;

		typedef std::string Member;
		typedef std::string Method;
		std::map<Method, std::set<Member>> kohesion;

		void collect_member_references(CXCursor cursor);
		static CXChildVisitResult collect_member_references(CXCursor cursor, CXCursor parent, CXClientData data);
		void reportKohesion(std::ostream &os) const;
		std::string formatMember(CXCursor cursor);
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

#endif
