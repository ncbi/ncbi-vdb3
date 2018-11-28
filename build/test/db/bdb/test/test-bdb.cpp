#include "../bdb.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::BDB::HelloBdb() == "Hello from BDB" )
    {
        cout << "BDB test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
