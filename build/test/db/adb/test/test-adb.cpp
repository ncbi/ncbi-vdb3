#include "../adb.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::ADB::HelloAdb() == "HelloTest from ADB" )
    {
        cout << "ADB test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
