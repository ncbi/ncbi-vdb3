
#include <bdb/bdb.hpp>

using namespace NCBI::VDB3;

PlatformMessage
NCBI::VDB3::BDB::HelloBdb()
{
    return HelloMsg() + " from BDB";
}
