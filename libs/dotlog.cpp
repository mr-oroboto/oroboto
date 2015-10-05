/**
 * dotlog.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "dotlog.h"
#include "logger.h"

/**
 * @param char * logName
 */
DotLog::DotLog(const char *logName)
{
	if (strlen(logName) > 512)
	{
		Logger::getInstance()->error("DotLog(%s)::ctor: log name is too long!", logName);
		throw;
	}

	char logPrefix[1024];
	char logFilePath[1024];

	snprintf(logPrefix, sizeof(logPrefix), "DotLog(%s)", logName);

	_logger = new Logger(logPrefix);

	snprintf(logFilePath, sizeof(logFilePath), "%s.dlg", logName);

	if ((_fLog = fopen(logFilePath, "w+")) == NULL)
	{
		_logger->error("ctor: could not open log for writing!");
		throw;
	}
}

DotLog::~DotLog()
{
	if (_fLog)
	{
		fclose(_fLog);
	}
}

/**
 * @param double ts				event timestamp
 * @param double x				x co-ordinate
 * @param double y				y co-ordinate
 * @param PositionColour colour	colour code
 * @param bool waypoint			is this a waypoint?
 *
 * @return void
 */
void DotLog::log(double ts, double x, double y, DotLogPositionColour::Colour colour, bool waypoint)
{
	unsigned char hexColour[3] = {'\0', '\0', '\0'};

	switch (colour)
	{
		case DotLogPositionColour::RED:
			hexColour[0] = 0xff;
			break;

		case DotLogPositionColour::GREEN:
			hexColour[1] = 0xff;
			break;

		case DotLogPositionColour::BLUE:
			hexColour[2] = 0xff;
			break;
	}

	fprintf(_fLog, "%.2f\t%.2f\t%.2f %x%x%x%s\n", ts, x, y, hexColour[0], hexColour[1], hexColour[2], waypoint ? " 1" : " 0");
}
