
#include <adb/adb.hpp>

#include "adb-priv.hpp"

using namespace NCBI::VDB3;

TestMessage
NCBI::VDB3::ADB::HelloAdb()
{
    return HelloTest() + " from ADB";
}
