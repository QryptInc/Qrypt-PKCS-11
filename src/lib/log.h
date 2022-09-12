#ifndef _QRYPTOKI_LOG_H
#define _QRYPTOKI_LOG_H

#include <syslog.h>     // LOG_ERR, etc.

/* Logging errors */
#define ERROR_MSG(...) qryptokiLog(LOG_ERR, __VA_ARGS__);

/* Logging warnings */
#define WARNING_MSG(...) qryptokiLog(LOG_WARNING, __VA_ARGS__);

/* Logging information */
#define INFO_MSG(...) qryptokiLog(LOG_INFO, __VA_ARGS__);

/* Logging debug information */
#define DEBUG_MSG(...) qryptokiLog(LOG_DEBUG, __VA_ARGS__);

/* Function definitions */
void qryptokiLog(const int level, const char* format, ...);

#endif /* !_QRYPTOKI_LOG_H */

