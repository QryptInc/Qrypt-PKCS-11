/*
 * Copyright (c) 2010 SURFnet bv
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
 log.cpp

 Implements logging functions.
 *****************************************************************************/

#include <stdarg.h>   // va_*
#include <stdio.h>    // fprintf, fflush, stderr
#include <sstream>    // std::stringstream
#include <vector>     // std::vector

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

void qryptWrapperLog(const int loglevel, const char* functionName, const char* fileName, const int lineNo, const char* format, ...)
{
	int maxLogLevel;

	const char *maxLogLevel_c_str = getenv("QRYPT_LOG_LEVEL");
	if(maxLogLevel_c_str == NULL) {
		maxLogLevel = DEFAULT_MAX_LOG_LEVEL;
	} else {
		try {
			maxLogLevel = std::stoi(std::string(maxLogLevel_c_str));
		} catch (...) {}
	}

	if(loglevel > maxLogLevel) return;

	std::stringstream prepend;

	// prepend << fileName << "(" << lineNo << ")";
	// (void) functionName;
	// prepend << ":";
	// prepend << " ";
	// prepend << functionName << ": ";

	prepend << logLevelToString(loglevel) << ": ";

	// Print the format to a log message
	std::vector<char> logMessage;
	va_list args;

	logMessage.resize(4096);

	va_start(args, format);
	vsnprintf(&logMessage[0], 4096, format, args);
	va_end(args);

	// And log it
	// syslog(loglevel, "%s%s", prepend.str().c_str(), &logMessage[0]);

	fprintf(stderr, "%s%s\n", prepend.str().c_str(), &logMessage[0]);
	fflush(stderr);
}

