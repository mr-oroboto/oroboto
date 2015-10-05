/**
 * odo.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 */

#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>

#include "odo.h"
#include "gpio.h"
#include "logger.h"
#include "motorlib.h"
#include "pwmlib.h"

extern "C" void * gOdoThread(void *arg)
{
    Odometer *o = static_cast<Odometer *>(arg);
    return o->thread();
}

/**
 * @param unsigned int wheelLeftGPIOA		GPIO # for left wheel optical encoder output A
 * @param unsigned int wheelLeftGPIOB		GPIO # for left wheel optical encoder output B
 * @param unsigned int wheelRightGPIOA		GPIO # for right wheel optical encoder output A
 * @param unsigned int wheelRightGPIOB		GPIO # for right wheel optical encoder output B
 * @param unsigned int wheelRadius			the radius of the wheel in centimeters
 */
Odometer::Odometer(unsigned int wheelLeftGPIOA, unsigned int wheelLeftGPIOB, unsigned int wheelRightGPIOA, unsigned int wheelRightGPIOB, unsigned int wheelRadius)
{
	_logger     = new Logger("Odometer");

	_leftGPIOA  = wheelLeftGPIOA;
	_leftGPIOB  = wheelLeftGPIOB;
	_rightGPIOA = wheelRightGPIOA;
	_rightGPIOB = wheelRightGPIOB;

	_wheelRadius = wheelRadius;

	_logger->notice("ctor: Configuring left wheel GPIOs %d and %d", _leftGPIOA, _leftGPIOB);
	_logger->notice("ctor: Configuring right wheel GPIOs %d and %d", _rightGPIOA, _rightGPIOB);

	// Configure the GPIOs to be inputs
	gpio_export(_leftGPIOA);
	gpio_export(_leftGPIOB);
	gpio_export(_rightGPIOA);
	gpio_export(_rightGPIOB);

	gpio_set_direction(_leftGPIOA, INPUT_PIN);
	gpio_set_direction(_leftGPIOB, INPUT_PIN);
	gpio_set_direction(_rightGPIOA, INPUT_PIN);
	gpio_set_direction(_rightGPIOB, INPUT_PIN);

	// The GPIOs should interrupt on both rising and falling edges
	gpio_set_edge(_leftGPIOA, EDGE_BOTH);
	gpio_set_edge(_leftGPIOB, EDGE_BOTH);
	gpio_set_edge(_rightGPIOA, EDGE_BOTH);
	gpio_set_edge(_rightGPIOB, EDGE_BOTH);

	reset();
}

/**
 * dtor
 */
Odometer::~Odometer()
{
	_logger->debug("dtor");
}

/**
 * Odometer::run - run the state machine thread
 *
 * @return void
 */
void Odometer::run()
{
	pthread_t tOdo;

	_logger->debug("run: creating odometry thread ...");
	pthread_create(&tOdo, NULL, gOdoThread, this);

	// pthread_join(tOdo);
}

/**
 * Odometer::reset - reset the odometer
 *
 * @return void
 */
void Odometer::reset()
{
	_odoLeft    = _odoRight    = 0;
	_errorsLeft = _errorsRight = 0;

//	_bRun   = false;
	_bError = false;
}

/**
 * Odometer::thread - the thread function
 *
 * This waits for transitions (rising or falling edges) on the GPIO file descriptors and then uses gray code to determine
 * which wheel has turned in which direction.
 *
 * @return void *
 */
void * Odometer::thread()
{
	struct pollfd fdset[4];
	int           nfds = 4, i;
	int           fdLeftA, fdLeftB, fdRightA, fdRightB;
	char          buf[8];
	unsigned int  nRead;

	unsigned char level;
	unsigned char levelLeftA     = 0, levelLeftB     = 0, levelRightA     = 0, levelRightB     = 0;
	unsigned char levelLeftAPrev = 0, levelLeftBPrev = 0, levelRightAPrev = 0, levelRightBPrev = 0;

	reset();

	fdLeftA  = gpio_fd_open(_leftGPIOA, O_RDONLY);
	fdLeftB  = gpio_fd_open(_leftGPIOB, O_RDONLY);

	fdRightA = gpio_fd_open(_rightGPIOA, O_RDONLY);
	fdRightB = gpio_fd_open(_rightGPIOB, O_RDONLY);

	if (fdLeftA < 0 || fdLeftB < 0 || fdRightA < 0 || fdRightB < 0)
	{
		_logger->error("thread: failed to open GPIO file descriptors");

		_bError = true;
		_bRun   = false;

		pthread_exit((void*)-1);
	}

	_bRun = true;

	while (_bRun)
	{
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd     = fdLeftA;
		fdset[0].events = POLLPRI;
		fdset[1].fd     = fdLeftB;
		fdset[1].events = POLLPRI;
		fdset[2].fd     = fdRightA;
		fdset[2].events = POLLPRI;
		fdset[3].fd     = fdRightB;
		fdset[3].events = POLLPRI;

		// Wait for an interrupt
		if (poll(fdset, nfds, -1 /* no timeout */) < 0)
		{
			_logger->error("thread: poll() returned < 0");

			_bError = true;
			_bRun   = false;

			pthread_exit((void*)-1);
		}

		// Determine which GPIO interrupted
		if ((fdset[0].revents & POLLPRI) || (fdset[1].revents & POLLPRI) || (fdset[2].revents & POLLPRI) || (fdset[3].revents & POLLPRI))
//		if (fdset[0].revents & POLLPRI || fdset[1].revents & POLLPRI)
		{
			for (i = 0; i < nfds; i++)
			{
				lseek(fdset[i].fd, 0, 0);

				// Read the current state of the GPIO input (high or low)
				if ((nRead = read(fdset[i].fd, buf, sizeof(buf))) <= 0)
				{
					_logger->error("thread: failed to read GPIO FD %d on interrupt", i);

					_bError = true;
					_bRun   = false;

					pthread_exit((void*)-1);
				}

				buf[nRead] = '\0';
				if (atoi(buf))
				{
					level = 1;
				}
				else
				{
					level = 0;
				}

				switch (i)
				{
					case 0:
						levelLeftA = level;
						break;

					case 1:
						levelLeftB = level;
						break;

					case 2:
						levelRightA = level;
						break;

					case 3:
						levelRightB = level;
						break;
				}
			}

			// What's happening to the left wheel?
			if (levelLeftA ^ levelLeftBPrev)
			{
				_odoLeft++;
			}
			if (levelLeftB ^ levelLeftAPrev)
			{
				_odoLeft--;
			}

//			_logger->debug("thread: LEFT [%d %d] => %d", levelLeftA, levelLeftB, _odoLeft);

			// We can't see state transitions on both channels, that's an invalid transition for the gray code.
			if (levelLeftA != levelLeftAPrev && levelLeftB != levelLeftBPrev)
			{
				_logger->notice("thread: LEFT odometry error, multiple transitions");
				_errorsLeft++;
			}

			// What's happening to the right wheel?
			//
			// NOTE: If the wrong encoder sensor is connected to the wrong GPIO, this will count backwards when the
			//       wheel is turning forwards.
			if (levelRightA ^ levelRightBPrev)
			{
				_odoRight++;
			}
			if (levelRightB ^ levelRightAPrev)
			{
				_odoRight--;
			}

//			_logger->debug("thread: RIGHT [%d %d] => %d", levelRightA, levelRightB, _odoRight);

			// We can't see state transitions on both channels, that's an invalid transition for the gray code.
			if (levelRightA != levelRightAPrev && levelRightB != levelRightBPrev)
			{
				_logger->notice("thread: RIGHT odometry error, multiple transitions");
				_errorsRight++;
			}

			levelLeftAPrev  = levelLeftA;
			levelLeftBPrev  = levelLeftB;
			levelRightAPrev = levelRightA;
			levelRightBPrev = levelRightB;
		}
	}

	_logger->debug("thread: exiting");

	gpio_fd_close(fdLeftA);
	gpio_fd_close(fdLeftB);
	gpio_fd_close(fdRightA);
	gpio_fd_close(fdRightB);

	_bRun = false;

	pthread_exit((void*)0);
}

/**
 * Odometer::stop - stop the state machine thread
 *
 * @return void
 */
void Odometer::stop()
{
	_bRun = false;
}

/**
 * Odometer::getOdometry
 *
 * @param int * wheelLeft	the odometer count for the left wheel
 * @param int * wheelRight	the odometer count for the right wheel
 *
 * @return void
 */
void Odometer::getOdometry(int *wheelLeft, int *wheelRight)
{
	// @todo: lock
	*wheelLeft  = _odoLeft;
	*wheelRight = _odoRight;
}

/**
 * Odometer::getDistance (returns values in centimeters)
 *
 * @param double *	wheelLeft	the distance travelled by the left wheel
 * @param double *	wheelRight	the distance travelled by the right wheel
 *
 * @return void
 */
void Odometer::getDistance(double *wheelLeft, double *wheelRight)
{
	*wheelLeft  = (2.0*M_PI*_wheelRadius*_odoLeft)  / ODO_TICKS_PER_REVOLUTION;
	*wheelRight = (2.0*M_PI*_wheelRadius*_odoRight) / ODO_TICKS_PER_REVOLUTION;
}

/**
 * Odometer::getErrorCount
 *
 * Get the number of invalid gray code transitions seen for each wheel.
 *
 * @param unsigned int * wheelLeft		error count for left wheel
 * @param unsigned int * wheelRight		error count for right wheel
 *
 * @return void
 */
void Odometer::getErrorCount(unsigned int *wheelLeft, unsigned int *wheelRight)
{
	// @todo: lock
	*wheelLeft  = _errorsLeft;
	*wheelRight = _errorsRight;
}

/**
 * Odometer::getRunning - is the thread running?
 *
 * @return bool
 */
bool Odometer::getRunning()
{
	return _bRun;
}

/**
 * Odometer::getError - was there an error that stopped the thread?
 *
 * @return bool
 */
bool Odometer::getError()
{
	return _bError;
}

/**
 * Odometer::getTimeToDistance
 *
 * Measures how long it takes to spin one wheel n revolutions at PWM speed percentage X.
 *
 * This is used to calibrate the motors and determine how a PWM speed percentage corresponds to a given cm/s.
 *
 * NOTE: This is a blocking call and does not run in a separate thread.
 *
 * @param bool wheelLeft
 * @param bool forward
 * @param int  revolutions
 * @param int  speed
 *
 * @return unsigned long
 */
unsigned long Odometer::getTimeToDistance(bool wheelLeft, bool forward, int revolutions, int speed)
{
	struct pollfd 	fdset[2];
	int           	nfds = 2, i;
	int           	fdA, fdB;
	int          	gpioA, gpioB;
	char          	buf[8];
	unsigned int  	nRead;

	unsigned char 	ucState;
	unsigned char 	ucA = 0,     ucB = 0;
	unsigned char 	ucLastA = 0, ucLastB = 0;

	int           	nCalibration = 0;

	struct timeval 	tStart, tEnd;

	if (_bRun)
	{
		_logger->error("getTimeToDistance: calibration cannot be run when the odometry thread is running");
		return 0;
	}

	reset();

	if (wheelLeft)
	{
		_logger->debug("getTimeToDistance: running calibration for left wheel");

		gpioA = _leftGPIOA;
		gpioB = _leftGPIOB;
	}
	else
	{
		_logger->debug("getTimeToDistance: running calibration for right wheel");

		gpioA = _rightGPIOA;
		gpioB = _rightGPIOB;
	}

	_logger->debug("getTimeToDistance: opening the following GPIOs: %d and %d\n", gpioA, gpioB);

	fdA  = gpio_fd_open(gpioA, O_RDONLY);
	fdB  = gpio_fd_open(gpioB, O_RDONLY);

	if (fdA < 0 || fdB < 0)
	{
		_logger->error("getTimeToDistance: failed to open GPIO file descriptors");
		return 0;
	}

	if (gettimeofday(&tStart, NULL) < 0)
	{
		_logger->error("getTimeToDistance: could not get start time");
		return 0;
	}

	if (wheelLeft)
	{
		if (forward)
		{
			motor_forward(MOTOR_LEFT, pwm_speed(speed));
		}
		else
		{
			motor_reverse(MOTOR_LEFT, pwm_speed(speed));
		}
	}
	else
	{
		if (forward)
		{
			motor_forward(MOTOR_RIGHT, pwm_speed(speed));
		}
		else
		{
			motor_reverse(MOTOR_RIGHT, pwm_speed(speed));
		}
	}

	bool firstRevolution = true;

	while ( ! _bError && abs(nCalibration) < (static_cast<int>(ODO_TICKS_PER_REVOLUTION)*revolutions))
	{
//		_logger->debug("getTimeToDistance: waiting for interrupt");

		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = fdA;
		fdset[0].events = POLLPRI;
		fdset[1].fd = fdB;
		fdset[1].events = POLLPRI;

		if (poll(fdset, nfds, -1 /* no timeout */) < 0)
		{
			_logger->error("getTimeToDistance: timed out waiting for interrupt");
			break;
		}

		if ((fdset[0].revents & POLLPRI) || (fdset[1].revents & POLLPRI))
		{
			for (i = 0; i < nfds; i++)
			{
				lseek(fdset[i].fd, 0, 0);

				if ((nRead = read(fdset[i].fd, buf, sizeof(buf))) <= 0)
				{
					_logger->error("getTimeToDistance: failed to read GPIO FD %d on interrupt", i);

					_bError = true;
					break;
				}

				buf[nRead] = '\0';
				if (atoi(buf))
				{
					ucState = 1;
				}
				else
				{
					ucState = 0;
				}

				switch (i)
				{
					case 0:
						ucA = ucState;
						break;

					case 1:
						ucB = ucState;
						break;
				}
			}

			/**
			 * What's happening to the wheel?
			 *
			 * NOTE: These depend on which GPIOs each sensor is connected to. If the wires are around the wrong way
			 * we will count backwards when we the wheels are going forwards and vice versa.
			 */
			if (ucA ^ ucLastB)
			{
				nCalibration++;
			}
			if (ucB ^ ucLastA)
			{
				nCalibration--;
			}

//			_logger->debug("getTimeToDistance: CALIBRATION [%d %d] => %d", ucA, ucB, nCalibration);

			/**
			 * We can't see state transitions on both channels, that's an invalid transition for the gray code. However,
			 * it IS valid to see this on the FIRST tick because we don't actually know our starting state.
			 */
			if (ucA != ucLastA && ucB != ucLastB && ! firstRevolution)
			{
				_logger->error("getTimeToDistance: invalid transition detected");
				_bError = true;
			}

			ucLastA  = ucA;
			ucLastB  = ucB;

			firstRevolution = false;
		}
	}

	if (gettimeofday(&tEnd, NULL) < 0)
	{
		_logger->error("getToDistance: could not get end time");
		_bError = true;
	}

	gpio_fd_close(fdA);
	gpio_fd_close(fdB);

	if (_bError)
	{
		_logger->notice("getTimeToDistance: calibration completed with errors");
		return 0;
	}

    if (tEnd.tv_usec < tStart.tv_usec)
    {
        int nsec = (tStart.tv_usec - tEnd.tv_usec) / 1000000 + 1;
        tStart.tv_usec -= 1000000 * nsec;
        tStart.tv_sec  += nsec;
    }

    if (tEnd.tv_usec - tStart.tv_usec > 1000000)
    {
        int nsec = (tStart.tv_usec - tEnd.tv_usec) / 1000000;
        tStart.tv_usec += 1000000 * nsec;
        tStart.tv_sec  -= nsec;
    }

    // tv_usec should be positive.
    return ((tEnd.tv_sec - tStart.tv_sec) * 1000) + ((tEnd.tv_usec - tStart.tv_usec) / 1000);
}
