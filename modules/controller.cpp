/**
 * controller.cpp
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

#include "controller.h"
#include "../libs/motorlib.h"
#include "../libs/pwmlib.h"
#include "../libs/led.h"
#include "../libs/logger.h"
#include "../libs/dotlog.h"
#include "../libs/odo.h"
#include "../libs/poseprovider.h"

Controller::Controller()
{
	_totalTime  = 0;
	_bSimulation = false;	// (debug) simulate movement rather than actually turning wheels

	_logger = new Logger("Controller");
	_odo    = new Odometer(LEFT_WHEEL_ENCODER_GPIO_A, LEFT_WHEEL_ENCODER_GPIO_B, RIGHT_WHEEL_ENCODER_GPIO_A, RIGHT_WHEEL_ENCODER_GPIO_B, CONTROLLER_WHEELRADIUS);

	_dotLogPosition = new DotLog("position");

	reset();
	_odo->run();
}

Controller::~Controller()
{
	_logger->notice("dtor: destroying");
}

/**
 * Controller::reset()
 *
 * These values should only be reset once. They set the robot position as (0,0) with a heading of 0 radians.
 */
void Controller::reset()
{
	_fHeadingRef = 0.0;
    _fHeadingError = 0.0;
    _fHeadingErrorPrev = 0.0;
    _fHeadingErrorIntegral = 0.0;

    _fPosXRef = 0.0;
    _fPosYRef = 0.0;

    _currentPose.x = 0.0;
    _currentPose.y = 0.0;
    _currentPose.heading = 0.0;
    _currentPose.timestamp = 0;

    resetDistance();
}

/**
 * Controller::resetDistance()
 *
 * These values must be reset for transit between each waypoint.
 */
void Controller::resetDistance()
{
    _fDistLeft = 0.0;
    _fDistLeftPrev = 0.0;
    _fDistRight = 0.0;
    _fDistRightPrev = 0.0;
    _fDistTotal = 0.0;
    _fDistTotalPrev = 0.0;
}

/**
 * Controller::goToPosition
 *
 * @param double x					desired waypoint x co-ord
 * @param double y					desired waypoing y co-ord
 * @param double fPosXStated		believed current x co-ord
 * @param double fPosYStated		believed current y co-ord
 *
 * @return void
 */
void Controller::goToPosition(double x, double y, double fPosXStated, double fPosYStated)
{
	double  		fTargetVectorMagnitude;             	// current distance between where we think we are and the waypoint
	double  		fTargetVectorMagnitudeLast = 0;			// last distance between where we think we were and the waypoint
	double  		fTargetVectorMagnitudeInitial = 0;		// initial distance between where we think we are and the waypoint
	double			fTargetVectorMagnitudeAtStartOfDrift;	// if we begin to get further from the target (rather than closer, perhaps due to a turning circle) what was our distance to the target when this started?
	unsigned int  	nConsecutiveIncreasingDistance = 0;		// number of times the distance has increased rather than decrease (used to stop out of controlness)
	bool			bApproachingTarget = false;				// once we start approaching the target don't forget it

	double       	fVelocityLeft = 0, fVelocityRight = 0;	// current velocity of left and right wheels

	Led    			ledHealth(1), ledProximity(3);

	ledHealth.off();
	ledProximity.off();

	// Reset the distance counters that are used in the PID loop, they are relative to our last (this) waypoint
	resetDistance();

	_logger->notice("goToPosition: --- START ---\nGoing to position (%.2f, %.2f) from current position (%.2f, %.2f) and heading [%.2f] ...", x, y, _currentPose.x, _currentPose.y, _currentPose.heading);

	_fPosXRef = x;
	_fPosYRef = y;

	// We need to keep our heading as close to this reference heading as possible to reach the waypoint
	_fHeadingRef = getHeading(_fPosXRef, _fPosYRef, _currentPose.x, _currentPose.y, _currentPose.heading);
//	_fHeadingRef = getHeading(_fPosXRef, _fPosYRef, fPosXStated, fPosYStated, _fHeadingCurrent);

	_logger->notice("goToPosition: new heading required is [%.2f]", _fHeadingRef);

	// Start (and reset) the odometry thread
	_odo->reset();

	int          	iteration = 0;
	bool          	bFirstIteration = true;

   	struct timeval 	tvNow, tvLast;
   	int           	nsec;
   	unsigned long 	delayms, totaldelayms = 0;	// time since last iteration and total time in this waypoint

   	gettimeofday(&tvLast, NULL);

	while (true)
    {
		/**
		 * Recalculate our reference heading every now and again as our current position changes.
		 */
		if (iteration % 10 == 0)
		{
			_logger->notice("goToPosition: recalculating heading based on current position...");
			_fHeadingRef = getHeading(_fPosXRef, _fPosYRef, _currentPose.x, _currentPose.y, _currentPose.heading);
			_logger->notice("new heading is %.2f", _fHeadingRef);
		}

		gettimeofday(&tvNow, NULL);

	    if (tvNow.tv_usec < tvLast.tv_usec)
	    {
	        nsec = (tvLast.tv_usec - tvNow.tv_usec) / 1000000 + 1;
	        tvLast.tv_usec -= 1000000 * nsec;
	        tvLast.tv_sec  += nsec;
	    }

	    if (tvNow.tv_usec - tvLast.tv_usec > 1000000)
	    {
	        nsec = (tvLast.tv_usec - tvNow.tv_usec) / 1000000;
	        tvLast.tv_usec += 1000000 * nsec;
	        tvLast.tv_sec  -= nsec;
	    }

	    // How long did we sleep since the last iteration? Required for derivatives.
	    delayms = ((tvNow.tv_sec - tvLast.tv_sec) * 1000) + ((tvNow.tv_usec - tvLast.tv_usec) / 1000);
	    totaldelayms += delayms;	// total time in this waypoint segment
	    _totalTime   += delayms;	// total time transiting altogether
	    tvLast        = tvNow;

	    _currentPose.timestamp = _totalTime;

	    // How long have we slept? This is required for our integral and derivative PID values.
    	double dt = delayms / 1000.0;

        ledHealth.strobe();

       	_logger->notice("goToPosition: %lu --- ITERATION %d --- \nreference: (%.2f, %.2f, distance: %.2f) at heading %.2f (total runtime: %lu) (dt: %.2f)", delayms, iteration, _fPosXRef, _fPosYRef, fTargetVectorMagnitudeInitial, _fHeadingRef, totaldelayms, dt);

        // How far has each wheel travelled? This is total distance since odo reset (start of waypoint).
        if (_bSimulation)
        {
        	// Don't use the odometer, see what our model predicts. This can be used to determine how accurate the odometer and drive train is.
            _fDistLeft  += dt * fVelocityLeft;
            _fDistRight += dt * fVelocityRight;
        }
        else
        {
	        _odo->getDistance(&_fDistLeft, &_fDistRight);
	    }

        _fDistTotal = (_fDistLeft + _fDistRight) / 2.0;

    	_logger->notice("goToPosition: distL[%.2f] distR[%.2f] dist[%.2f] distPrev[%.2f]", _fDistLeft, _fDistRight, _fDistTotal, _fDistTotalPrev);

        // How far have we travelled in this last iteration?
        double fDistDelta      = _fDistTotal - _fDistTotalPrev;
        double fDistLeftDelta  = _fDistLeft  - _fDistLeftPrev;
        double fDistRightDelta = _fDistRight - _fDistRightPrev;

    	// Position has changed based on the distance travelled at the previous heading
        _currentPose.x += fDistDelta * cos(_currentPose.heading);
    	_currentPose.y += fDistDelta * sin(_currentPose.heading);

    	// Update the heading as it has changed based on the distance travelled too
    	_currentPose.heading += ((fDistRightDelta - fDistLeftDelta) / CONTROLLER_WHEELBASE);

        // Ensure our heading remains sane
    	_currentPose.heading = atan2(sin(_currentPose.heading), cos(_currentPose.heading));

    	_fDistTotalPrev  = _fDistTotal;
    	_fDistLeftPrev   = _fDistLeft;
    	_fDistRightPrev  = _fDistRight;

        // What's the error between our required heading and our heading?
		double fHeadingErrorRaw = _fHeadingRef - _currentPose.heading;
		_fHeadingError = atan2(sin(fHeadingErrorRaw), cos(fHeadingErrorRaw));

		// How fast should we proceed forward? (Anything below 5 results in non-movement due to inertia)
		double fForwardVelocity = 9.0;              // 5.0 works as well but undershoots

       	// What is the magnitude of the vector between us and our target?
       	fTargetVectorMagnitude = sqrt(((_fPosXRef - _currentPose.x)*(_fPosXRef - _currentPose.x)) + ((_fPosYRef - _currentPose.y)*(_fPosYRef - _currentPose.y)));

       	if (bFirstIteration)
       	{
       	    fTargetVectorMagnitudeInitial = fTargetVectorMagnitude;
       	}

    	_logger->notice("goToPosition: heading[%.4f, e: %.6f] posX[%.2f] posY[%.2f] distToTarget[%.2f] distToTargetLast[%.2f]", _currentPose.heading, _fHeadingError, _currentPose.x, _currentPose.y, fTargetVectorMagnitude, fTargetVectorMagnitudeLast);

    	/**
    	 * @todo: Come up with a better way of determining whether we should consider ourself "close enough" to the waypoint.
    	 */
    	if ( ! bFirstIteration)
    	{
    	    if (fTargetVectorMagnitude >= fTargetVectorMagnitudeLast)
    	    {
    	    	if (nConsecutiveIncreasingDistance == 0)
    	    	{
    	    		fTargetVectorMagnitudeAtStartOfDrift = fTargetVectorMagnitude;
    	    	}

    	        nConsecutiveIncreasingDistance++;

    	        double fDistanceCoveredSinceStartOfDrift = fTargetVectorMagnitude - fTargetVectorMagnitudeAtStartOfDrift;

    	        if (fDistanceCoveredSinceStartOfDrift >= (0.25 * fTargetVectorMagnitudeInitial))
//    	        if (nConsecutiveIncreasingDistance > MAX_ITERATIONS_OF_INCREASING_TARGET_VECTOR_BEFORE_TERMINATION)
    	        {
		    	    _logger->notice("goToPosition: Distance to target has increased!\ngoToPosition: --- END ABNORMAL ---\n");
    	            break;
    	        }
    	    }
    	    else
    	    {
    	        nConsecutiveIncreasingDistance = 0;
    	    }
    	}

    	if (fTargetVectorMagnitude < 10 || bApproachingTarget)
    	{
    		bApproachingTarget = true;

    		ledProximity.on();
    	    _logger->notice("goToPosition: Approaching target, slowing down.");
    	    fForwardVelocity = 3.0;                 // 3.0 works as well, safer
    	}

//      _logger->notice("goToPosition: x(%.2f)\ty(%.2f)\ttheta(%.2f)", _fPosXCurrent, _fPosYCurrent, _fHeadingCurrent);

    	if (fTargetVectorMagnitude <= 5)
    	{
    	    _logger->notice("goToPosition: You have arrived at your destination!\ngoToPosition: --- END ---\n");
            _dotLogPosition->log((_totalTime / 1000.0), _currentPose.x, _currentPose.y, DotLog::DotLogPositionColour::RED, true);
    	    break;
    	}
    	else
    	{
            _dotLogPosition->log((_totalTime / 1000.0), _currentPose.x, _currentPose.y, bApproachingTarget ? DotLog::DotLogPositionColour::RED : DotLog::DotLogPositionColour::BLACK);
    	}

		fTargetVectorMagnitudeLast = fTargetVectorMagnitude;

//    	if (fabs(fPosXCurrent - fPosXRef) < (CONTROLLER_WHEELBASE/2.0) && fabs(fPosYCurrent - fPosYRef) < (CONTROLLER_WHEELBASE/2.0))
//    	{
//    		printf("You have arrived at your destination (non-vector)!\n");
//    		break;
//    	}

    	// Maintain the PID variables
    	double fHeadingErrorDerivative = (_fHeadingError - _fHeadingErrorPrev) / dt;
    	_fHeadingErrorIntegral        += (_fHeadingError * dt);
    	_fHeadingErrorPrev             = _fHeadingError;

    	double pidP = CONTROLLER_PID_PROPORTIONAL * _fHeadingError;
    	double pidI = CONTROLLER_PID_INTEGRAL     * _fHeadingErrorIntegral;
    	double pidD = CONTROLLER_PID_DERIVATIVE   * fHeadingErrorDerivative;

    	// PID, this gives us the control signal, u, this is required angular velocity
    	double u = pidP + pidI + pidD;

    	// Angular velocity gives us new wheel velocities. We assume a constant forward velocity for simplicity.
    	fVelocityRight = ((2.0*fForwardVelocity) + (u*CONTROLLER_WHEELBASE)) / (2.0*CONTROLLER_WHEELRADIUS);   // cm/s
		fVelocityLeft  = ((2.0*fForwardVelocity) - (u*CONTROLLER_WHEELBASE)) / (2.0*CONTROLLER_WHEELRADIUS);   // cm/s

		_logger->notice("goToPosition: P[%.6f] I[%.6f] D[%.6f] u[%.6f] -> required velocities (left,right) are (%.2f,%.2f)", pidP, pidI, pidD, u, fVelocityLeft, fVelocityRight);

		// These velocities are relative, work out the required velocity per wheel.
//		float vLeft  = (fVelocityLeft  / (fabs(fVelocityLeft) + fabs(fVelocityRight))) * 100.0;
//		float vRight = (fVelocityRight / (fabs(fVelocityLeft) + fabs(fVelocityRight))) * 100.0;
//
//		int   dLeft  = static_cast<int>(fabs(vLeft));
//		int   dRight = static_cast<int>(fabs(vRight));
//
//		printf("Required relative velocities (left,right) are (%.2f,%.2f) -> (%d,%d)\n", vLeft, vRight, dLeft, dRight);

		int nLeftPWM  = convertVelocityToPWMPercentage(true, fVelocityLeft);
		int nRightPWM = convertVelocityToPWMPercentage(false, fVelocityRight);

		// Apply the new velocities to the motors
		if ( ! _bSimulation)
		{
			if (fVelocityLeft < 0.0)
			{
				motor_reverse(MOTOR_LEFT, pwm_speed(nLeftPWM));
			}
			else
			{
				motor_forward(MOTOR_LEFT, pwm_speed(nLeftPWM));
			}

			if (fVelocityRight < 0.0)
			{
				motor_reverse(MOTOR_RIGHT, pwm_speed(nRightPWM));
			}
			else
			{
				motor_forward(MOTOR_RIGHT, pwm_speed(nRightPWM));
			}
		}

		usleep(50000);      // 50ms
//		usleep(300000);     // 300 ms
//		usleep(1000000);    // 1s

    	/**
    	 * time passes ... wheels respond to new control signal and begin moving at new velocities
    	 *
    	 *  These wheel velocities turn the wheels and influence the distance travelled and heading
    	 *  which is recalculated in the next iteration.
    	 */

    	bFirstIteration = false;
    	iteration++;
    }

    bot_stop();

    ledHealth.off();
}

/**
 * Controller::convertVelocityToPWMPercentage
 *
 * Based on on-the-floor testing which using the odometer, resulted in a curve that plots PWM duty cycle
 * against velocity, determine the DC required to turn the specified motor at the desired velocity.
 *
 * @param bool  	leftMotor
 * @param double 	fVelocity
 *
 * @return int
 */
int Controller::convertVelocityToPWMPercentage(bool leftMotor, double fVelocity)
{
	float fPercentage, fWorkingVelocity;
	int   nPercentage;

	/**
	 * The requested velocity could be negative, which means we turn the motor backwards. This is fine, but our curves
	 * are for forward velocities only. We return an absolute PWM speed, the caller turns it into a positive or neg.
	 */
	fWorkingVelocity = fabs(fVelocity);

	if (fWorkingVelocity > CONTROLLER_MAX_VELOCITY)
	{
		_logger->notice("convertVelocityToPWMPercentage: requested velocity (%.2f) is above maximum (%.2f), capping at max", fVelocity, CONTROLLER_MAX_VELOCITY);
		fWorkingVelocity = CONTROLLER_MAX_VELOCITY;
	}

	if (leftMotor)
	{
		fPercentage = 10.0 * (fWorkingVelocity + 0.2833) / 1.103;
	}
	else
	{
		fPercentage = 10.0 * (fWorkingVelocity + 0.2395) / 1.0263;
	}

	nPercentage = static_cast<int>(round(fPercentage));

//	_logger->debug("convertVelocityToPWMPercentage: velocity %.2f converts to PWM DC %d (%.2f)", fVelocity, nPercentage, fPercentage);

	return nPercentage;
}

/**
 * Controller::getHeading
 *
 * Translates current pose to (0,0, 0 rad) and works out the heading required to get to global co-ordinate (x,y)
 * from a given current global co-ordinate.
 *
 * @param double toX				destination global x co-ordinate
 * @param double toY				destination global y co-ordinate
 * @param double fromX				current global x co-ordinate
 * @param double fromY				current global y co-ordinate
 * @param double fCurrentHeading	current heading
 *
 * @return double
 */
double Controller::getHeading(double toX, double toY, double fromX, double fromY, double fCurrentHeading)
{
	double fRad = fCurrentHeading;

	double x = toX - fromX;
	double y = toY - fromY;

	_logger->notice("getHeading: to global   (%.2f,%.2f) from global: (%.2f,%.2f) currentHeading: %.2f rad", toX, toY, fromX, fromY, fCurrentHeading);
	_logger->notice("getHeading: to relative (%.2f,%.2f)", x, y);

	if (x <= 0.01 && x >= -0.01)
	{
		if (y < 0.0)
		{
			fRad = (double)(3.0*M_PI)/2.0;
		}
		else if (y > 0.0)
		{
			fRad = M_PI/2.0;
		}
		else
		{
			_logger->notice("getHeading: leaving at current heading");
		}
	}
	else
	{
		fRad = atan(y / x);

		if (x > 0)
		{
			fRad += (2.0*M_PI);

			if (fRad > (2.0*M_PI))
			{
				fRad -= (2.0*M_PI);
			}
		}
		else if (x < 0)
		{
			fRad += M_PI;
		}
	}

	return fRad;
}

/**
 * Get the current pose of the robot.
 *
 * @todo: critical section around these
 *
 * @return Pose
 */
Pose Controller::getCurrentPose()
{
	return _currentPose;
}
