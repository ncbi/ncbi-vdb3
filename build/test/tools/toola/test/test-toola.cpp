#include "../toola.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::ToolA().run() == 0 )
    {
        cout << "ToolA test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
