#pragma once

#include "Visitor.hpp"
#include "graphviz.hpp"

namespace metric
{


class Kohesion :
        public Visitor
{
    private:
        graphviz::Graph bgraph{};

        void collect_member_references(CXCursor cursor);
        static CXChildVisitResult collect_member_references(CXCursor cursor, CXCursor parent, CXClientData data);
        void reportKohesion(std::ostream &os) const;
    public:
        std::string name() const override;

        const graphviz::Graph& graph() const override;

        CXChildVisitResult visit(
                CXCursor cursor,
                CXCursor parent) override;

        void report(std::ostream &) const override;
};


}
