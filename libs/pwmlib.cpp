/**
 * pwmlib.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "pwmlib.h"
#include "logger.h"
#include "sysfslib.h"

/**
 * Initialise the PWM subsystem.
 *
 * @return int
 */
int pwm_init()
{
    /**
     * Ask CapeMgr to load the PWM module virtual cape.
     */
    return sysfs_write(SLOTS_FILE, PWM_DRIVER);
}

/**
 * Enable a PWM channel, the channel is set to the default period and polarity and pulled high (ie. 100% DC).
 *
 * The PWM subsystem must have first been enabled with pwm_init()
 *
 * @see pwm_init()
 *
 * @param int pwm    the PWM channel to enable (0..3)
 *
 * @return int
 */
int pwm_enable(int pwm)
{
    const char *pwmDriver;

    switch (pwm)
    {
        case 0:
        	pwmDriver = PWM_0_DRIVER;
            break;

        case 1:
        	pwmDriver = PWM_1_DRIVER;
            break;

        case 2:
        	pwmDriver = PWM_2_DRIVER;
            break;

        case 3:
        	pwmDriver = PWM_3_DRIVER;
            break;

        default:
        	Logger::getInstance()->error("pwm::pwm_enable: unknown PWM");
            return -1;
    }

    if (sysfs_write(SLOTS_FILE, pwmDriver) == 0)
    {
        // Pull the PWM pin high by default
        pwm_set_period(pwm, PWM_DEFAULT_PERIOD);
        pwm_set_polarity(pwm, PWM_DEFAULT_POLARITY);
        pwm_pull(pwm, PWM_DIRECTION_HIGH);
    }
    else
    {
    	Logger::getInstance()->error("pwm::pwm_enable: failed to enable PWM channel %d", pwm);
        return -1;
    }

    return 0;
}

/**
 * Set the period of a PWM channel that has been enabled with pwm_enable()
 *
 * @see pwm_enable()
 *
 * @param int pwm      the PWM channel
 * @param int period   the period (in nanoseconds)
 *
 * @return int
 */
int pwm_set_period(int pwm, int period)
{
    char str_period[16];
    char str_ctrl_file[1024];

    const char *pwm_ctrl_file_prefix = pwm_get_ctrl_file_prefix(pwm);

    if ( ! pwm_ctrl_file_prefix)
    {
    	Logger::getInstance()->error("pwm::pwm_set_period: unknown PWM");
        return -1;
    }

    snprintf(str_ctrl_file, sizeof(str_ctrl_file), "%s/%s", pwm_ctrl_file_prefix, PERIOD_FILE);
    snprintf(str_period, sizeof(str_period), "%d", period);

    return sysfs_write(str_ctrl_file, str_period);
}

/**
 * Set the duty cycle of a PWM channel that has been enabled with pwm_enable()
 *
 * @see pwm_enable()
 *
 * @param int pwm      the PWM channel
 * @param int duty     the DC expressed in nanoseconds (ie. for a 50% DC, set this to half of the period)
 *
 * @return int
 */
int pwm_set_duty(int pwm, int duty)
{
    char str_duty[16];
    char str_ctrl_file[1024];

    const char *pwm_ctrl_file_prefix = pwm_get_ctrl_file_prefix(pwm);

    if ( ! pwm_ctrl_file_prefix)
    {
    	Logger::getInstance()->error("pwm::pwm_set_duty: unknown PWM");
        return -1;
    }

    snprintf(str_ctrl_file, sizeof(str_ctrl_file), "%s/%s", pwm_ctrl_file_prefix, DUTY_FILE);
    snprintf(str_duty, sizeof(str_duty), "%d", duty);

    return sysfs_write(str_ctrl_file, str_duty);
}

/**
 * Set the polarity of a PWM channel that has been enabled with pwm_enable()
 *
 * @see pwm_enable()
 *
 * @param int pwm      the PWM channel
 * @param int polarity the polarity (0 or 1)
 *
 * @return int
 */
int pwm_set_polarity(int pwm, int polarity)
{
    char str_polarity[16];
    char str_ctrl_file[1024];

    const char *pwm_ctrl_file_prefix = pwm_get_ctrl_file_prefix(pwm);

    if ( ! pwm_ctrl_file_prefix)
    {
    	Logger::getInstance()->error("pwm::pwm_set_polarity: unknown PWM");
        return -1;
    }

    snprintf(str_ctrl_file, sizeof(str_ctrl_file), "%s/%s", pwm_ctrl_file_prefix, POLARITY_FILE);
    snprintf(str_polarity, sizeof(str_polarity), "%d", polarity);

    return sysfs_write(str_ctrl_file, str_polarity);
}

/**
 * Pull a PWM output completely high or low.
 *
 * @param int pwm       the PWM channel
 * @param int direction PWM_DIRECTION_LOW or PWM_DIRECTION_HIGH
 *
 * @dragon This assumes the PWM is using the default polarity and period!
 *
 * @return int
 */
int pwm_pull(int pwm, int direction)
{
    switch (direction)
    {
        case PWM_DIRECTION_LOW:
            pwm_set_duty(pwm, 0);
            break;

        case PWM_DIRECTION_HIGH:
            pwm_set_duty(pwm, PWM_DEFAULT_PERIOD);
            break;

        default:
        	Logger::getInstance()->error("pwm::pwm_pull: unknown direction");
            return -1;
    }

    return 0;
}

/**
 * Get the sysfs control file prefix for a specific PWM channel.
 *
 * @param int pwm      the PWM channel
 *
 * @return const char *
 */
const char * pwm_get_ctrl_file_prefix(int pwm)
{
    const char *pwm_ctrl_file_prefix = NULL;

    switch (pwm)
    {
        case 0:
            pwm_ctrl_file_prefix = PWM_DIR_PREFIX PWM_0_DIR;
            break;

        case 1:
            pwm_ctrl_file_prefix = PWM_DIR_PREFIX PWM_1_DIR;
            break;

        case 2:
            pwm_ctrl_file_prefix = PWM_DIR_PREFIX PWM_2_DIR;
            break;

        case 3:
            pwm_ctrl_file_prefix = PWM_DIR_PREFIX PWM_3_DIR;
            break;

        default:
        	Logger::getInstance()->error("pwm::pwm_get_ctrl_file_prefix: unknown PWM");
            return NULL;
    }

    return pwm_ctrl_file_prefix;
}

/**
 * A helper to convert DC as a percentage to DC in nS based on the default period.
 *
 * @param int dcPercent
 *
 * @return int
 */
int pwm_speed(int dcPercent)
{
    if (dcPercent < 0 || dcPercent > 100)
    {
    	Logger::getInstance()->error("pwm::pwm_speed: invalid duty cycle percentage");
        return -1;
    }

    int duty = (int)(((float)(100 - dcPercent) / 100.0)*(float)PWM_DEFAULT_PERIOD);
//  int duty = (int)(((float)(dcPercent) / 100.0)*(float)PWM_DEFAULT_PERIOD);

    return duty;
}

