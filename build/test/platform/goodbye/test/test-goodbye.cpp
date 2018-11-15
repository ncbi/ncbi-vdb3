#include "../goodbye.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::GoodbyeTest() == "GoodbyeTest" )
    {
        cout << "Goodbye test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
