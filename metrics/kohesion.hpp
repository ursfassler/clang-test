#pragma once

#include "Visitor.hpp"
#include <map>
#include <set>
#include <vector>


class Kohesion :
        public Visitor
{
    private:
        typedef std::vector<std::string> Path;
        typedef Path Member;
        typedef Path Method;
        std::map<Method, std::set<Member>> graph;

        void collect_member_references(CXCursor cursor);
        static CXChildVisitResult collect_member_references(CXCursor cursor, CXCursor parent, CXClientData data);
        void reportKohesion(std::ostream &os) const;
    public:
        CXChildVisitResult visit(
                CXCursor cursor,
                CXCursor parent) override;

        void report(std::ostream &) const override;
};
