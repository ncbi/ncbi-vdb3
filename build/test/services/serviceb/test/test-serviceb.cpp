#include "../serviceb.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::ServiceB().start() == 0 )
    {
        cout << "ServiceB test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
