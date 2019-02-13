#ifndef DIO_H
#define DIO_H

#include <rsdio_export.h>
#include "rsdio_global.h"

#ifdef __cplusplus
extern "C" {
#endif

RSDIO_EXPORT bool initDio(const char *initFile);
RSDIO_EXPORT int digitalRead(int dio, int pin);
RSDIO_EXPORT int digitalWrite(int dio, int pin, bool state);
RSDIO_EXPORT int setOutputMode(int dio, OutputMode mode);
RSDIO_EXPORT const char *getLastDioError();

#ifdef __cplusplus
}
#endif
#endif