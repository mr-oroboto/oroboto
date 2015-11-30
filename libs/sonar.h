/**
 * sonar.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Implements a separate thread to take "SONAR" measurements using a MAXSONAR-EZ1 ultrasonic transducer
 * attached to one of the BeagleBone Black's ADC channels.
 *
 * There is no magic here about the MAXSONAR-EZ1, it could be anything so long as the voltage measured
 * on the ADC is relative to the distance measured by the transducer.
 */

#ifndef _SONAR_H_INCLUDED
#define _SONAR_H_INCLUDED

#define SONAR_ADC_CHANNEL 4								// which ADC does the ultrasonic transducer live on?
#define SONAR_SAMPLES_PER_MEASUREMENT 32				// how many samples to take per "measurement" (will be averaged by ADC)
#define SONAR_ADC_DISTANCE_CORRECTION_FACTOR 1.0		// environment specific fuzz factor
#define SONAR_SLEEP_PER_MEASUREMENT_USEC 100000

class Logger;
class DotLog;
class PoseProvider;

class Sonar
{
	private:
		unsigned int	_nADC;
		unsigned int	_nSamples;
		PoseProvider *  _poseProvider;

		bool			_bRun;       // Should the SONAR thread exit?
		bool			_bMeasure;   // Should the SONAR thread make measurements?

		Logger *		_logger;
		DotLog * 		_dotLogSonar;

	public:
		Sonar(const unsigned int nADC, const unsigned int nSamples, PoseProvider *poseProvider);
		~Sonar();

		void    run();
		void    stop();
		void *  sonarThread();

		void	calibrate(unsigned int maxIterations = 200, unsigned int periodUs = 50);

		void	startMeasuring();
		void	stopMeasuring();

		bool    isRunning();
		bool	isMeasuring();
};

#endif // _SONAR_H_INCLUDED
