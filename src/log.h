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
 log.h

 Implements logging functions.
 *****************************************************************************/

#ifndef _QRYPT_WRAPPER_LOG_H
#define _QRYPT_WRAPPER_LOG_H

#include <syslog.h>     // LOG_ERR, etc.

/* Logging errors */
#ifndef _WIN32
#define ERROR_MSG(...) qryptWrapperLog(LOG_ERR, __func__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define ERROR_MSG(...) qryptWrapperLog(LOG_ERR, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

/* Logging warnings */
#ifndef _WIN32
#define WARNING_MSG(...) qryptWrapperLog(LOG_WARNING, __func__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define WARNING_MSG(...) qryptWrapperLog(LOG_WARNING, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

/* Logging information */
#ifndef _WIN32
#define INFO_MSG(...) qryptWrapperLog(LOG_INFO, __func__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define INFO_MSG(...) qryptWrapperLog(LOG_INFO, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

/* Logging debug information */
#ifndef _WIN32
#define DEBUG_MSG(...) qryptWrapperLog(LOG_DEBUG, __func__, __FILE__, __LINE__, __VA_ARGS__);
#else
#define DEBUG_MSG(...) qryptWrapperLog(LOG_DEBUG, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

/* Function definitions */
void qryptWrapperLog(const int loglevel, const char* functionName, const char* fileName, const int lineNo, const char* format, ...);

#endif /* !_QRYPT_WRAPPER_LOG_H */

