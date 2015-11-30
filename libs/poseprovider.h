/**
 * poseprovider.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Interface for a provider of current position and heading information.
 */

#ifndef _POSEPROVIDER_H_INCLUDED
#define _POSEPROVIDER_H_INCLUDED

struct Pose {
	double 			x;
	double 			y;
	double 			heading;
	unsigned long	timestamp;
};

class PoseProvider
{
	public:
		virtual Pose getCurrentPose() = 0;
};

#endif // _POSEPROVIDER_H_INCLUDED
