/**
 * led.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Trivial abstraction to simplify use of BeagleBone LEDs.
 */

#ifndef _LED_H_INCLUDED
#define _LED_H_INCLUDED

#define LED_FILE_PREFIX "/sys/class/leds/beaglebone:green:usr"

class Logger;

class Led
{
	private:
		unsigned int	_nLed;

		bool			_bRun;      // Should the LED thread exit?
		bool			_bOn;       // Is the LED currently on?

		Logger *		_logger;

		void	hijack();

	public:
		Led(unsigned int nLed);
		~Led();

		void    run();
		void    stop();
		void *  pulseThread();

		void    on();
		void    off();
		void    strobe();

		bool    isRunning();
};

#endif // _LED_H_INCLUDED
