#include "b.h"


namespace nsb
{
namespace sub
{
namespace
{

void anonB()
{
}

}

B::B(nsa::A &a_) :
    a{a_}
{
}

void B::funcB()
{
    a.funcA();
}


}
}
