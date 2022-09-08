#ifndef _QRYPTOKI_LOG_H
#define _QRYPTOKI_LOG_H

#include <syslog.h>     // LOG_ERR, etc.

/* Logging errors */
#ifndef _WIN32
#define ERROR_MSG(...) qryptokiLog(LOG_ERR, __VA_ARGS__);
#else
#define ERROR_MSG(...) qryptokiLog(LOG_ERR, __VA_ARGS__);
#endif

/* Logging warnings */
#ifndef _WIN32
#define WARNING_MSG(...) qryptokiLog(LOG_WARNING, __VA_ARGS__);
#else
#define WARNING_MSG(...) qryptokiLog(LOG_WARNING, __VA_ARGS__);
#endif

/* Logging information */
#ifndef _WIN32
#define INFO_MSG(...) qryptokiLog(LOG_INFO, __VA_ARGS__);
#else
#define INFO_MSG(...) qryptokiLog(LOG_INFO, __VA_ARGS__);
#endif

/* Logging debug information */
#ifndef _WIN32
#define DEBUG_MSG(...) qryptokiLog(LOG_DEBUG, __VA_ARGS__);
#else
#define DEBUG_MSG(...) qryptokiLog(LOG_DEBUG, __VA_ARGS__);
#endif

/* Function definitions */
void qryptokiLog(const int level, const char* format, ...);

#endif /* !_QRYPTOKI_LOG_H */

