#ifndef __VISITOR__HPP__
#define __VISITOR__HPP__

#include <clang-c/Index.h>
#include <string>
#include <ostream>
#include <map>
#include <vector>

class Visitor
{
	public:
		class Result
		{
			public:
				CXCursor cursor;
				std::string key;
				std::string value;
		};

		typedef std::multimap<std::string, Result> ResultContainer;

  public:
    static std::string namespace_for(CXCursor cursor);
    static std::vector<std::string> path_for(CXCursor cursor);
    static std::string print(const std::vector<std::string>&);
    bool ignore(CXCursor cursor) const;

	public:
		virtual ~Visitor();

		virtual const std::string & get_name() const = 0;
		virtual const std::string & get_id() const = 0;

		virtual CXChildVisitResult visit(
				CXCursor cursor,
				CXCursor parent) = 0;

		virtual void report(std::ostream &) const = 0;
		virtual void collect(ResultContainer &) const = 0;

		static CXChildVisitResult visitor_recursive(
				CXCursor cursor,
				CXCursor parent,
				CXClientData data);
};

Visitor * visitor_cast(CXClientData data);

#endif
