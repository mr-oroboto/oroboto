/**
 * gpio.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "gpio.h"
#include "sysfslib.h"

/****************************************************************
 * gpio_export
 ****************************************************************/

int gpio_export(unsigned int gpio)
{
    char exportPath[16];

    snprintf(exportPath, sizeof(exportPath), "%d", gpio);

    return sysfs_write(GPIO_DIR_PREFIX "/export", exportPath);
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/

int gpio_unexport(unsigned int gpio)
{
    char exportPath[16];

    snprintf(exportPath, sizeof(exportPath), "%d", gpio);

    return sysfs_write(GPIO_DIR_PREFIX "/unexport", exportPath);
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/

int gpio_set_direction(unsigned int gpio, PIN_DIRECTION outFlag)
{
    char gpioFile[1024];

    snprintf(gpioFile, sizeof(gpioFile), GPIO_DIR_PREFIX "/gpio%d/direction", gpio);

	if (outFlag == OUTPUT_PIN)
	{
	    return sysfs_write(gpioFile, "out");
	}
	else
	{
	    return sysfs_write(gpioFile, "in");
	}
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/

int gpio_set_value(unsigned int gpio, PIN_VALUE value)
{
    char gpioFile[1024];

    snprintf(gpioFile, sizeof(gpioFile), GPIO_DIR_PREFIX "/gpio%d/value", gpio);

	if (value == LOW)
	{
	    return sysfs_write(gpioFile, "0");
	}
	else
	{
	    return sysfs_write(gpioFile, "1");
	}
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/

int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	char ch;
	char gpioFile[1024];

	snprintf(gpioFile, sizeof(gpioFile), GPIO_DIR_PREFIX "/gpio%d/value", gpio);

	if ( ! sysfs_read(gpioFile, &ch, 1))
	{
		perror("gpio::get_value");
		return -1;
	}

	if (ch != '0')
	{
		*value = 1;
	}
	else
	{
		*value = 0;
	}

	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, INTERRUPT_EDGE edge)
{
    char gpioFile[1024];

    snprintf(gpioFile, sizeof(gpioFile), GPIO_DIR_PREFIX "/gpio%d/edge", gpio);

    switch (edge)
    {
        case EDGE_RISING:
			return sysfs_write(gpioFile, "rising");

        case EDGE_FALLING:
			return sysfs_write(gpioFile, "falling");

        case EDGE_BOTH:
			return sysfs_write(gpioFile, "both");

        default:
            perror("gpio::set_edge");
    }

	return -1;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio, int flags = O_RDONLY | O_NONBLOCK)
{
    char gpioFile[1024];

    snprintf(gpioFile, sizeof(gpioFile), GPIO_DIR_PREFIX "/gpio%d/value", gpio);

	return sysfs_open_read(gpioFile, flags);
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

void gpio_fd_close(int fd)
{
	sysfs_close(fd);
}
