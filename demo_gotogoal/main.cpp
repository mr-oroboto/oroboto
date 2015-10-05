/**
 * main.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Simple demo to show basic go-to-goal strategy using a BeagleBone Black differential drive robot.
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "motorlib.h"
#include "adclib.h"
#include "controller.h"

int main(int argc, char *argv[])
{
	int i, nWayPoints = 4;

	float waypoints[][4] = {
		{45.0, 0.0},
		{90.0, 45.0},
		{0.0, 45.0},
		{1.0, 1.0}
	};

    motor_init();
    adc_init();

	Controller c;

	for (i = 0; i < nWayPoints; i++)
	{
		double fPosXStated = 0;
		double fPosYStated = 0;

		if (i != 0)
		{
			fPosXStated = waypoints[i-1][0];
			fPosYStated = waypoints[i-1][1];
		}

		bot_stop();
		sleep(1);
		c.goToPosition(waypoints[i][0],waypoints[i][1], fPosXStated, fPosYStated);
	}

	return 0;
}
