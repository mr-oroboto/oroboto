/**
 * logger.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"

/**
 * @param char * logPrefix
 */
Logger::Logger(const char *logPrefix)
{
	if (strlen(logPrefix) > 255)
	{
		fprintf(stderr, "Logger::ctor: logPrefix [%s] is too long!", logPrefix);
		throw;
	}

	snprintf(_logPrefix, sizeof(_logPrefix), "%s", logPrefix);
}

/**
 * singleton
 *
 * @return Logger *
 */
Logger * Logger::getInstance()
{
	static Logger *	_singleton;

	if ( ! _singleton)
	{
		_singleton = new Logger("");
	}

	return _singleton;
}

/**
 * @param char * format
 * @param ...
 *
 * @return void
 */
void Logger::debug(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (strcmp(_logPrefix, "") != 0)
	{
		fprintf(stderr, "%s::", _logPrefix);
	}

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}

/**
 * @param char * format
 * @param ...
 *
 * @return void
 */
void Logger::notice(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (strcmp(_logPrefix, "") != 0)
	{
		fprintf(stderr, "%s::", _logPrefix);
	}

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}

/**
 * @param char * format
 * @param ...
 *
 * @return void
 */
void Logger::error(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (strcmp(_logPrefix, "") != 0)
	{
		fprintf(stderr, "%s::", _logPrefix);
	}

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}
