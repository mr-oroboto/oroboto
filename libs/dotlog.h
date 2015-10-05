/**
 * dotlog.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * DotLog log file creator.
 */

#ifndef _DOTLOG_H_INCLUDED
#define _DOTLOG_H_INCLUDED

class Logger;

class DotLog
{
	private:
		FILE * 		_fLog;
		Logger *	_logger;

	public:
		DotLog(const char *logName);
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
