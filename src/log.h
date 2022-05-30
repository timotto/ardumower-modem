#pragma once

#include <Arduino.h>

#ifndef LOG_CON
#define LOG_CON Serial
#endif

#define NONE    0x00
#define DBG     0x1F
#define INFO    0x0F
#define ERR     0x07
#define EMR     0x03
#define CRIT    0x01

#ifndef DEBUG_LEVEL
//#ifdef ESP_MODEM_SIM
#define DEBUG_LEVEL DBG
//#elif ESP_MODEM_TEST
//#define DEBUG_LEVEL DBG
//#else
//#define DEBUG_LEVEL INFO
//#endif
#endif

#define DEBUG(...)  LOG_CON.printf(__VA_ARGS__)
#define Log(X, ...) if((DEBUG_LEVEL & X) == X) \
                                  { \
                                    DEBUG("%.3f %d ", millis() / 1000.0, X); \
                                    DEBUG(__VA_ARGS__); \
                                    DEBUG("%s", "\r\n"); \
                                  }
