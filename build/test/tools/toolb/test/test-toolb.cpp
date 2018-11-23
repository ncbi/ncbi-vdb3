#include "../toolb.cpp"

#include <iostream>

using namespace std;

int main()
{
    if ( NCBI::VDB3::ToolB().run() == 0 )
    {
        cout << "ToolB test Passed" << endl;
        return 0;
    }
    else
    {
        return 1;
    }
}
