
#include <adb/adb.hpp>

#include "adb-priv.hpp"

using namespace NCBI::VDB3;

PlatformMessage
NCBI::VDB3::ADB::HelloAdb()
{
    return HelloMsg() + " from ADB";
}
