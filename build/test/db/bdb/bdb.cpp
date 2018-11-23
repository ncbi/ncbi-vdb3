
#include <bdb/bdb.hpp>

using namespace NCBI::VDB3;

TestMessage
NCBI::VDB3::BDB::HelloBdb()
{
    return HelloTest() + " from BDB";
}
