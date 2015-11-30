/**
 * dotlog.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * DotLog log file creator.
 */

#ifndef _DOTLOG_H_INCLUDED
#define _DOTLOG_H_INCLUDED

#define DOTLOG_SERVER "192.168.7.1"
#define DOTLOG_PORT   "50607"

class  Logger;
struct addrinfo;

class DotLog
{
	private:
		FILE * 				_fLog;
		Logger *			_logger;
		int        			_socket;
		struct addrinfo *	_dotLogServerList;
		struct addrinfo *	_dotLogServerAddr;


	public:
		DotLog(const char *logName, bool enableRemoteLogging = false);
		~DotLog();

		struct DotLogPositionColour
		{
			enum Colour {
				BLACK,
				RED,
				GREEN,
				BLUE
			};
		};

		void log(double ts, double x, double y, DotLogPositionColour::Colour colour = DotLogPositionColour::BLACK, bool waypoint = false);
};

#endif // _DOTLOG_H_INCLUDED
