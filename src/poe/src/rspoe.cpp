#include "rspoe.h"
#include "rspoeimpl.h"

// This macro is usually passed by CMake, but ensure it's available
#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

namespace rs {

// This function is compiled into the SHARED library, so it gets the Export attributes
RsPoe* createRsPoe() {
    return new RsPoeImpl();
}

const char* rsPoeVersion() {
    return RSSDK_VERSION_STRING;
}

} // namespace rs