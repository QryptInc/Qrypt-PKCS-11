#include <stdarg.h>   // va_*
#include <iostream>   // std::cerr

#include "log.h"

const int DEFAULT_MAX_LOG_LEVEL = LOG_INFO;

const char *logLevelToString(const int loglevel) {
	switch (loglevel) {
		case LOG_ERR:
			return "Error";
		case LOG_WARNING:
			return "Warning";
		case LOG_INFO:
			return "Info";
		case LOG_DEBUG:
			return "Debug";
		default:
			return "Unknown log level";
	}
}

void qryptokiLog(const int level, const char* format, ...)
{
	int max_level = DEFAULT_MAX_LOG_LEVEL;

	const char *max_level_c_str = getenv("QRYPT_LOG_LEVEL");
	if(max_level_c_str != NULL) {
		try {
			max_level = std::stoi(std::string(max_level_c_str));
		} catch (...) {}
	}

	if(level > max_level) return;

	// Print the format to a log message
	char message[4096];
	va_list args;

	va_start(args, format);
	vsnprintf(message, 4096, format, args);
	va_end(args);

	std::cerr << logLevelToString(level) << ": " << message << std::endl;
}

