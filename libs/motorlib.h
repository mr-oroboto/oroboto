/**
 * motorlib.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Abstraction for basic control of DC motors connected to a DRV8833 which is itself driven by the BeagleBone's PWMs.
 */

#ifndef MOTORLIB_H_INCLUDED
#define MOTORLIB_H_INCLUDED

#define MOTOR_LEFT          0
#define MOTOR_RIGHT         1

#define MOTOR_LEFT_PWM_A    1               // the PWMs that connect to the left motor's DRV8833 control signals, see pwmlib.h
#define MOTOR_LEFT_PWM_B    0

#define MOTOR_RIGHT_PWM_A   3               // the PWMs that connect to the right motor's DRV8833 control signals, see pwmlib.h
#define MOTOR_RIGHT_PWM_B   2

#define SPIN_DIRECTION_LEFT  0
#define SPIN_DIRECTION_RIGHT 1

void motor_init();
int  motor_forward(int motor, int speed);
int  motor_reverse(int motor, int speed);
int  motor_stop(int motor);

int  bot_forward(int speed);
int  bot_reverse(int speed);
void bot_stop();
int  bot_spin(int speed, int direction);

#endif // MOTORLIB_H_INCLUDED
