/**
 * controller.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * PID controller implementation.
 *
 * Some other stuff is currently embedded into this as well (SONAR sounding etc) which should be separated out.
 */

#ifndef _CONTROLLER_H_INCLUDED
#define _CONTROLLER_H_INCLUDED

#include "../libs/poseprovider.h"

#define CONTROLLER_PID_PROPORTIONAL 0.90        // contributes to stability and medium-rate responsiveness
#define CONTROLLER_PID_INTEGRAL 0.0005          // tracking and disturbance rejection (slow-rate responsiveness, may cause oscillations)
#define CONTROLLER_PID_DERIVATIVE 0.00          // fast-rate responsiveness, can cause overshoot: note 0.1 is safer

#define CONTROLLER_MAX_VELOCITY 10.0

#define CONTROLLER_WHEELBASE   9				// in centimeters
#define CONTROLLER_WHEELRADIUS 2                // in centimeters, ensure this matches

#define LEFT_WHEEL_ENCODER_GPIO_A	30
#define LEFT_WHEEL_ENCODER_GPIO_B	31
#define RIGHT_WHEEL_ENCODER_GPIO_A	66
#define RIGHT_WHEEL_ENCODER_GPIO_B	67

#define MAX_ITERATIONS_OF_INCREASING_TARGET_VECTOR_BEFORE_TERMINATION 8

class Odometer;
class Logger;
class DotLog;

struct Pose;

class Controller : public PoseProvider
{
	private:
		Pose	_currentPose;					// (believed) current x, y and heading

		double  _fHeadingRef;					// reference heading required to go from (believed) current pose to desired waypoint
		double  _fHeadingError;					// for PID proportional
		double  _fHeadingErrorPrev;				// for PID derivative
		double  _fHeadingErrorIntegral;			// for PID integral

		double  _fDistLeft;						// current distance travelled by left wheel in this waypoint segment
		double  _fDistLeftPrev;					// previous current distance travelled by left wheel in this waypoint segment
		double  _fDistRight;					// current distance travelled by right wheel in this waypoint segment
		double  _fDistRightPrev;				// previous current distance travelled by right wheel in this waypoint segment
		double  _fDistTotal;					// total distance travelled in this waypoint segment
		double  _fDistTotalPrev;				// previous total distance travelled in this waypoint segment

		double  _fPosXRef;						// x position of next desired waypoint
		double  _fPosYRef;						// y position of next desired waypoint

		unsigned long _totalTime;				// total time elapsed while transiting between waypoints (runtime)

		bool	_bSimulation;					// should we simulate movement (for testing model) or actually turn the wheels?

		void     reset();
		void     resetDistance();

		Odometer * _odo;
		Logger   * _logger;

		DotLog   * _dotLogPosition;

		int      convertVelocityToPWMPercentage(bool leftMotor, double fVelocity);

	public:
		Controller();
		~Controller();

		void        	goToPosition(double x, double y, double fPosXStated, double fPosYStated);
		double      	getHeading(double toX, double toY, double fromX, double fromY, double fCurrentHeading);

		Pose			getCurrentPose();
};

#endif // _CONTROLLER_H_INCLUDED
