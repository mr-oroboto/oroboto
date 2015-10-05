/**
 * odo.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * A simple odometer implementation.
 *
 * Uses the BBB GPIO pins as inputs to look for rising and falling edges from the optical encoders attached to the
 * wheels and then determines the distance traveled based on the number of revolutions turned and the wheel radius.
 */

#ifndef _ODO_H_INCLUDED
#define _ODO_H_INCLUDED

#define ODO_TICKS_PER_REVOLUTION 48.0

class Logger;

class Odometer
{
	private:
		// Physics
		unsigned int	_wheelRadius;	// in centimeters

		// The GPIO #s that we can read the optical encoder state for each wheel from
		unsigned int	_leftGPIOA, _leftGPIOB;
		unsigned int	_rightGPIOA, _rightGPIOB;

		// The actual odometry for each wheel
		int				_odoLeft;
		int				_odoRight;

		// The number of errors (since reset) for each wheel
		unsigned int	_errorsLeft;
		unsigned int	_errorsRight;

		// Should the odometry thread exit?
		bool			_bRun;

		// Did an error occur that stopped the thread?
		bool			_bError;

		Logger *		_logger;

	public:
		Odometer(unsigned int wheelLeftGPIOA, unsigned int wheelLeftGPIOB, unsigned int wheelRightGPIOA, unsigned int wheelRightGPIOB, unsigned int wheelRadius);
		~Odometer();

		void    reset();

		void    run();
		void    stop();
		void *  thread();

		unsigned long getTimeToDistance(bool wheelLeft, bool forward, int revolutions, int speed);

		void    getOdometry(int *wheelLeft, int *wheelRight);
		void    getDistance(double *wheelLeft, double *wheelRight);
		void    getErrorCount(unsigned int *wheelLeft, unsigned int *wheelRight);
		bool    getRunning();
		bool    getError();
};

#endif // _ODO_H_INCLUDED
