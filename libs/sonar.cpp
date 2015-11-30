/**
 * sonar.cpp
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

#include "sonar.h"
#include "logger.h"
#include "dotlog.h"
#include "adclib.h"
#include "poseprovider.h"

extern "C" void * gSonarThread(void *arg)
{
    Sonar *s = static_cast<Sonar *>(arg);
    return s->sonarThread();
}

/**
 * ctor
 *
 * @param const unsigned int nADC 		  the ADC to use
 * @param const unsigned int nSamples 	  the number of samples to take per measurement
 * @param PoseProvider *     poseProvider object that can provide heading and position information
 */
Sonar::Sonar(const unsigned int nADC, const unsigned int nSamples, PoseProvider *poseProvider)
{
	_nADC 		  = nADC;
	_nSamples	  = nSamples;
	_poseProvider = poseProvider;

	_logger      = new Logger("Sonar");
	_dotLogSonar = new DotLog("sonar", true);

	_bRun     = false;
	_bMeasure = false;
}

/**
 * dtor
 */
Sonar::~Sonar()
{
}

/**
 * Sonar::run - run the flash thread
 */
void Sonar::run()
{
	pthread_t tSonar;

	_logger->debug("run: starting SONAR thread ...");
	pthread_create(&tSonar, NULL, gSonarThread, this);
}

/**
 * Sonar::sonarThread - the thread body
 */
void * Sonar::sonarThread()
{
	_bRun = true;

	while (_bRun)
	{
		if (_bMeasure)
		{
	        // Take a reading from the uS transducer to get a distance to anything on our current heading
	        double fDistObstacle = static_cast<double>(adc_sample(_nADC, _nSamples)) * SONAR_ADC_DISTANCE_CORRECTION_FACTOR;

	        Pose currentPose = _poseProvider->getCurrentPose();

	        // Work out what the global position of that obstacle would be.
	        double fPosXObstacle = currentPose.x + (fDistObstacle * cos(currentPose.heading));
	        double fPosYObstacle = currentPose.y + (fDistObstacle * sin(currentPose.heading));

            _dotLogSonar->log((currentPose.timestamp / 1000.0), fPosXObstacle, fPosYObstacle, DotLog::DotLogPositionColour::BLACK, false);
		}

		usleep(SONAR_SLEEP_PER_MEASUREMENT_USEC);
	}

	_bRun = false;
	pthread_exit((void*)0);
}

/**
 * Sonar::stop - stop the SONAR thread
 */
void Sonar::stop()
{
	_logger->debug("stop: stopping SONAR thread");
	_bRun = false;
}

/**
 * Sonar::isRunning - is the SONAR thread running?
 */
bool Sonar::isRunning()
{
	return _bRun;
}

/**
 * Start the SONAR measurements (thread must be running)
 *
 * @return void
 */
void Sonar::startMeasuring()
{
	_bMeasure = true;
}

/**
 * Stop the SONAR measurements (thread does not exit)
 *
 * @return void
 */
void Sonar::stopMeasuring()
{
	_bMeasure = false;
}

/**
 * Is the SONAR thread measuring?
 *
 * @return bool
 */
bool Sonar::isMeasuring()
{
	return _bMeasure && _bRun;
}

/**
 * Calibration loop.
 *
 * Simply spins reading the ADC so that you can measure the values returned for certain distances (ie. of obstacles
 * that you place in front of the MAXSONAR-EZ1)
 *
 * @param unsigned int iterations	number of iterations to sample for
 * @param unsigned int periodUs		period to sleep between measurements (in microseconds)
 */
void Sonar::calibrate(unsigned int maxIterations, unsigned int periodUs)
{
	while (maxIterations--)
	{
        // Take a reading from the uS transducer to get a distance to anything in front of it
        double fDistObstacle = static_cast<double>(adc_sample(_nADC, _nSamples)) * SONAR_ADC_DISTANCE_CORRECTION_FACTOR;

    	_logger->notice("%.02f", fDistObstacle);

		usleep(periodUs);      // 50ms
	}
}
