#ifndef __METRIC_DIT__HPP__
#define __METRIC_DIT__HPP__

#include "Visitor.hpp"
#include "VisitorDescriptor.hpp"
#include <map>
#include <set>

class VisitorFactory;

class Metric_IT :
		public Visitor
{
	private:
		static const VisitorDescriptor DESCRIPTOR;

		void collect_base_classes(CXCursor cursor);

		static CXChildVisitResult collect_base_classes(
				CXCursor cursor,
				CXCursor parent,
				CXClientData data);

		std::map<std::string, std::set<std::string>> graph{};

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
