/**
 * motorlib.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Abstraction for basic control of DC motors connected to a DRV8833 which is itself driven by the BeagleBone's PWMs.
 *
 * Functions that accept a speed parameter pass it directly to the PWM subsystem, which expects speeds specified in
 * nanoseconds (to set the duty cycle of the PWM with relation to its period). Use the pwm_speed() function to get
 * these values.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "motorlib.h"
#include "pwmlib.h"

/**
 * Initialise the motor subsystem.
 */
void motor_init()
{
    pwm_init();

    // Control of two motors with the DRV8833 requires 4 PWM channels
    pwm_enable(0);
    pwm_enable(1);
    pwm_enable(2);
    pwm_enable(3);
}

/**
 * Drive a specific motor forward at a given speed.
 *
 * @see pwm_speed()
 *
 * @param int motor    MOTOR_LEFT or MOTOR_RIGHT
 * @param int speed    use pwm_speed() to get this value
 *
 * @return int
 */
int motor_forward(int motor, int speed)
{
    motor_stop(motor);

    switch (motor)
    {
        case MOTOR_LEFT:
            // slow decay, see DRV8833 datasheet for truth table
            pwm_pull(MOTOR_LEFT_PWM_A, PWM_DIRECTION_HIGH);
            pwm_set_duty(MOTOR_LEFT_PWM_B, speed);
            break;

        case MOTOR_RIGHT:
            // slow decay
            pwm_pull(MOTOR_RIGHT_PWM_A, PWM_DIRECTION_HIGH);
            pwm_set_duty(MOTOR_RIGHT_PWM_B, speed);
            break;

        default:
            fprintf(stderr, "motor_forward: unknown motor\n");
            return -1;
    }

    return 0;
}

/**
 * Drive a specific motor in reverse at a given speed.
 *
 * @see pwm_speed()
 *
 * @param int motor    MOTOR_LEFT or MOTOR_RIGHT
 * @param int speed    use pwm_speed() to get this value
 *
 * @return int
 */
int motor_reverse(int motor, int speed)
{
    motor_stop(motor);

    switch (motor)
    {
        case MOTOR_LEFT:
            // slow decay
            pwm_pull(MOTOR_LEFT_PWM_B, PWM_DIRECTION_HIGH);
            pwm_set_duty(MOTOR_LEFT_PWM_A, speed);
            break;

        case MOTOR_RIGHT:
            // slow decay
            pwm_pull(MOTOR_RIGHT_PWM_B, PWM_DIRECTION_HIGH);
            pwm_set_duty(MOTOR_RIGHT_PWM_A, speed);
            break;

        default:
            fprintf(stderr, "motor_reverse: unknown motor\n");
            return -1;
    }

    return 0;
}

/**
 * Brake/stop a given motor.
 *
 * @param int motor    MOTOR_LEFT or MOTOR_RIGHT
 */
int motor_stop(int motor)
{
    switch (motor)
    {
        case MOTOR_LEFT:
            pwm_pull(MOTOR_LEFT_PWM_A, PWM_DIRECTION_HIGH);
            pwm_pull(MOTOR_LEFT_PWM_B, PWM_DIRECTION_HIGH);
            break;

        case MOTOR_RIGHT:
            pwm_pull(MOTOR_RIGHT_PWM_A, PWM_DIRECTION_HIGH);
            pwm_pull(MOTOR_RIGHT_PWM_B, PWM_DIRECTION_HIGH);
            break;

        default:
            fprintf(stderr, "motor_stop: unknown motor\n");
            return -1;
    }

    return 0;
}

/**
 * Drive the bot forward at a given speed. If the motors are not well matched you may find this drifts off course.
 *
 * @see pwm_speed()
 *
 * @param int speed    use pwm_speed() to get this value
 *
 * @return int
 */
int bot_forward(int speed)
{
    motor_forward(MOTOR_LEFT, speed);
    motor_forward(MOTOR_RIGHT, speed);

    return 0;
}

/**
 * Drive the bot in reverse at a given speed. If the motors are not well matched you may find this drifts off course.
 *
 * @see pwm_speed()
 *
 * @param int speed    use pwm_speed() to get this value
 *
 * @return int
 */
int bot_reverse(int speed)
{
    motor_reverse(MOTOR_LEFT, speed);
    motor_reverse(MOTOR_RIGHT, speed);

    return 0;
}

/**
 * Stop the bot from moving.
 *
 * @return int
 */
void bot_stop()
{
    motor_stop(MOTOR_LEFT);
    motor_stop(MOTOR_RIGHT);
}

/**
 * Spin around on the spot (more or less, depends how well your motors are balanced!)
 *
 * @see pwm_speed()
 *
 * @param int speed      use pwm_speed() to get this value
 * @param int direction  SPIN_DIRECTION_LEFT or SPIN_DIRECTION_RIGHT
 *
 * @return int
 */
int bot_spin(int speed, int direction)
{
    bot_stop();

    switch (direction)
    {
        case SPIN_DIRECTION_LEFT:
            motor_forward(MOTOR_RIGHT, speed);
            motor_reverse(MOTOR_LEFT, speed);
            break;

        case SPIN_DIRECTION_RIGHT:
            motor_forward(MOTOR_LEFT, speed);
            motor_reverse(MOTOR_RIGHT, speed);
            break;

        default:
            fprintf(stderr, "bot_spin: invalid direction\n");
            return -1;
    }

    return 0;
}
