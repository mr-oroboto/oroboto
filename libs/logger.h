/**
 * logger.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Simple logging package.
 */

#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

class Logger
{
	private:
		char 			_logPrefix[256];

	public:
		Logger(const char *logPrefix);

		static Logger *	getInstance();

		void 			debug(const char *format, ...);
		void 			notice(const char *format, ...);
		void 			error(const char *format, ...);
};

#endif // _LOGGER_H_INCLUDED
