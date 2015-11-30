/**
 * led.cpp
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

#include "led.h"
#include "logger.h"
#include "sysfslib.h"

extern "C" void * gLedPulseThread(void *arg)
{
    Led *l = static_cast<Led *>(arg);
    return l->pulseThread();
}

/**
 * ctor
 *
 * @param unsigned int nLed 	the LED to use
 */
Led::Led(unsigned int nLed)
{
	_nLed   = nLed;
	_logger = new Logger("Led");

	if (_nLed > 3)
	{
		_logger->error("CLed: warning, LED number is too high!");
	}
	else
	{
		hijack();
	}

	_bRun = false;
}

/**
 * dtor
 */
Led::~Led()
{
}

/**
 * Led::run - run the flash thread
 */
void Led::run()
{
	pthread_t tLed;

	_logger->debug("run: creating LED thread ...");
	pthread_create(&tLed, NULL, gLedPulseThread, this);
}

/**
 * Led::pulseThread - the thread body
 */
void * Led::pulseThread()
{
	_bOn = false;

	_bRun = true;

	while (_bRun)
	{
		if (_bOn)
		{
			off();
		}
		else
		{
			on();
		}

		_bOn = !_bOn;

		sleep(1);
	}

	_bRun = false;
	pthread_exit((void*)0);
}

/**
 * Led::stop - stop the pulse thread
 */
void Led::stop()
{
	_bRun = false;
}

/**
 * Led::isRunning - is the pulse thread running?
 */
bool Led::isRunning()
{
	return _bRun;
}

/**
 * Led::on - turn the LED on
 */
void Led::on()
{
    char ledFile[256];

    snprintf(ledFile, sizeof(ledFile), "%s%d/brightness", LED_FILE_PREFIX, _nLed);
	sysfs_write(ledFile, "1");

	_bOn = true;
}

/**
 * Led::off - turn the LED off
 */
void Led::off()
{
    char ledFile[256];

    snprintf(ledFile, sizeof(ledFile), "%s%d/brightness", LED_FILE_PREFIX, _nLed);
	sysfs_write(ledFile, "0");

	_bOn = false;
}

/**
 * Led::strobe
 */
void Led::strobe()
{
	if (_bOn)
	{
		off();
	}
	else
	{
		on();
	}
}


/**
 * Led::hijack - hijack the LED from the BBB
 */
void Led::hijack()
{
    char ledFile[256];

    snprintf(ledFile, sizeof(ledFile), "%s%d/trigger", LED_FILE_PREFIX, _nLed);
	sysfs_write(ledFile, "none");
}
