/**
 * dotlog.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "dotlog.h"
#include "logger.h"

/**
 * @param char * logName
 * @param bool   enableRemoteLogging
 */
DotLog::DotLog(const char *logName, bool enableRemoteLogging)
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

	_dotLogServerList = NULL;
	_socket = -1;

	if (enableRemoteLogging)
	{
		struct addrinfo hints;
		int    ret;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family   = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;

		if ((ret = getaddrinfo(DOTLOG_SERVER, DOTLOG_PORT, &hints, &_dotLogServerList)) != 0)
		{
			_logger->error("ctor: could not resolve remote log host: %s", gai_strerror(ret));
			throw;
		}

		for (_dotLogServerAddr = _dotLogServerList; _dotLogServerAddr != NULL; _dotLogServerAddr = _dotLogServerAddr->ai_next)
		{
			if ((_socket = socket(_dotLogServerAddr->ai_family, _dotLogServerAddr->ai_socktype, _dotLogServerAddr->ai_protocol)) < 0)
			{
				_logger->error("ctor: could not create socket: %s", strerror(errno));
				continue;
			}

			break;
		}

		if (_socket < 0)
		{
			_logger->error("ctor: could not create any socket for communication with remote log host");
			throw;
		}
	}
}

DotLog::~DotLog()
{
	if (_fLog)
	{
		fclose(_fLog);
	}

	if (_dotLogServerList)
	{
		freeaddrinfo(_dotLogServerList);
	}

	if (_socket >= 0)
	{
		close(_socket);
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
	char          logLine[256];

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

	snprintf(logLine, sizeof(logLine), "%.2f\t%.2f\t%.2f %x%x%x%s\n", ts, x, y, hexColour[0], hexColour[1], hexColour[2], waypoint ? " 1" : " 0");
	fprintf(_fLog, "%s", logLine);

	if (_socket >= 0 && _dotLogServerAddr)
	{
		int bytesWritten = sendto(_socket, logLine, strlen(logLine), 0, _dotLogServerAddr->ai_addr, _dotLogServerAddr->ai_addrlen);

		if (bytesWritten < 0)
		{
			_logger->error("log: could not write log line to remote log server");
		}
	}
}
