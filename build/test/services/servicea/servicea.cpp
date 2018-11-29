
#include "servicea.hpp"

#include <iostream>

using namespace std;
using namespace NCBI::VDB3;

ServiceA::ServiceA()
{
}

int
ServiceA::start()
{
    cout << "ServiceA is starting" << endl;
    return 0;
}

