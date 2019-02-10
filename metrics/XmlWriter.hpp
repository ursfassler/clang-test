#pragma once

#include <string>
#include <vector>
#include <ostream>


namespace metric
{


class XmlWriter
{
    public:
        XmlWriter(std::ostream&);

        void startNode(const std::string&);
        void endNode();
        void attribute(const std::string& name, const std::string& value);

    private:
        std::ostream& stream;
        std::vector<std::string> path{};
        bool nodeIsOpen{false};

        std::string escape(const std::string &value);
};


}
