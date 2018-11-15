#include "../hello.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::HelloTest() == "HelloTest" )
    {
        cout << "Hello 2 test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
