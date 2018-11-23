#include "../servicea.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::ServiceA().start() == 0 )
    {
        cout << "ServiceA test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
