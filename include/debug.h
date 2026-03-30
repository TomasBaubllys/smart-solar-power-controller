#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <Arduino.h>

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#endif // DEBUG_H_INCLUDED