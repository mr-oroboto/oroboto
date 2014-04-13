/**
 * main.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Simple demo to show basic DC motor control using a BeagleBone, PWM and the DRV8833.
 */

#include <unistd.h>
#include "motorlib.h"
#include "pwmlib.h"

int main(int argc, char *argv[])
{
    motor_init();

	bot_stop();

	// Move forward at 50% speed for 2 seconds
	bot_forward(pwm_speed(50));
	sleep(2);

	// Spin around a bit, woohoo!
	bot_spin(pwm_speed(40), SPIN_DIRECTION_LEFT);
	sleep(1);
	bot_spin(pwm_speed(40), SPIN_DIRECTION_RIGHT);
	sleep(1);

	bot_stop();

	return 0;
}
