#pragma once

#include "Visitor.hpp"
#include "VisitorDescriptor.hpp"
#include <map>
#include <set>
#include <vector>

class VisitorFactory;




class Kohesion :
        public Visitor
{
    private:
        static const VisitorDescriptor DESCRIPTOR;

        typedef std::vector<std::string> Path;
        typedef Path Member;
        typedef Path Method;
        std::map<Method, std::set<Member>> graph;

        void collect_member_references(CXCursor cursor);
        static CXChildVisitResult collect_member_references(CXCursor cursor, CXCursor parent, CXClientData data);
        void reportKohesion(std::ostream &os) const;
    public:
        const std::string & get_name() const override;
        const std::string & get_id() const override;

        CXChildVisitResult visit(
                CXCursor cursor,
                CXCursor parent) override;

        void report(std::ostream &) const override;

        static void register_in(VisitorFactory &);
};
