/**
 * gpio.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2015
 *
 * Trivial abstraction to simplify use of BeagleBone GPIOs.
 */

#ifndef _GPIO_H_INCLUDED
#define _GPIO_H_INCLUDED

#define GPIO_DIR_PREFIX     "/sys/class/gpio"

enum PIN_DIRECTION
{
	INPUT_PIN  = 0,
	OUTPUT_PIN = 1
};

enum PIN_VALUE
{
	LOW  = 0,
	HIGH = 1
};

enum INTERRUPT_EDGE
{
	EDGE_RISING  = 0,
	EDGE_FALLING = 1,
	EDGE_BOTH    = 2
};

int 	gpio_export(unsigned int gpio);
int 	gpio_unexport(unsigned int gpio);
int 	gpio_set_direction(unsigned int gpio, PIN_DIRECTION outFlag);
int 	gpio_set_value(unsigned int gpio, PIN_VALUE value);
int 	gpio_get_value(unsigned int gpio, unsigned int *value);
int 	gpio_set_edge(unsigned int gpio, INTERRUPT_EDGE edge);
int 	gpio_fd_open(unsigned int gpio, int flags);
void 	gpio_fd_close(int fd);

#endif // _GPIO_H_INCLUDED
