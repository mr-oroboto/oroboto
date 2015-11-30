/**
 * calibrate.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Calibrate use of MAXSONAR-EZ1 by determining what ADC returns for given distances.
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adclib.h"
#include "sonar.h"

int main(int argc, char *argv[])
{
    adc_init();

	Sonar *sonar = new Sonar(SONAR_ADC_CHANNEL, SONAR_SAMPLES_PER_MEASUREMENT, NULL);

	sonar->calibrate();

	return 0;
}
