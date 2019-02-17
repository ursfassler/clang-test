#pragma once

#include "a.h"

namespace nsb::sub
{


class B
{
    public:
        B(nsa::A&);

        void funcB();

    private:
        nsa::A& a;

};


}
