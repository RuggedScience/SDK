#ifndef POE_H
#define POE_H

#include <poe/rspoe_export.h>
#include "poe_global.h"

#ifdef __cplusplus
extern "C" {
#endif

RSPOE_EXPORT bool initPoe(const char *initFile);
RSPOE_EXPORT PoeState getPortState(int port);
RSPOE_EXPORT int setPortState(int port, PoeState state);
RSPOE_EXPORT const char* getLastPoeError();

#ifdef __cplusplus
}
#endif
#endif