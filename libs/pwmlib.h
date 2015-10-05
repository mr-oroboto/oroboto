/**
 * pwmlib.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Abstraction for basic control of BeagleBone's PWMs.
 *
 * You may need to change the following constants depending on your BBB:
 *
 * PWM_DIR_PREFIX
 * PWM_0_DIR, PWM_1_DIR, PWM_2_DIR, PWM_3_DIR
 */

#ifndef _PWMLIB_H_INCLUDED
#define _PWMLIB_H_INCLUDED

#define PWM_DRIVER          "am33xx_pwm"                // the name of the PWM cape
#define PWM_0_DRIVER        "bone_pwm_P9_14"            // the PWM on P9_14
#define PWM_1_DRIVER        "bone_pwm_P9_16"            // the PWM on P9_16
#define PWM_2_DRIVER        "bone_pwm_P9_21"            // the PWM on P9_21
#define PWM_3_DRIVER        "bone_pwm_P9_22"            // the PWM on P9_22

#define PWM_DIR_PREFIX      "/sys/devices/ocp.2/"       // trailing slash is required, the '2' may vary depending on your BBB
#define PWM_0_DIR           "pwm_test_P9_14.15"         // the '15' may vary depending on your BBB
#define PWM_1_DIR           "pwm_test_P9_16.16"         // the '16' may vary depending on your BBB
#define PWM_2_DIR           "pwm_test_P9_21.17"         // the '17' may vary depending on your BBB
#define PWM_3_DIR           "pwm_test_P9_22.18"         // the '18' may vary depending on your BBB

#define PERIOD_FILE         "period"
#define DUTY_FILE           "duty"
#define POLARITY_FILE       "polarity"

#define PWM_DEFAULT_PERIOD      500000                  // 2Khz
#define PWM_DEFAULT_POLARITY    0                       // when polarity = 0, a DC of 0 leads to 0V and a DC of PERIOD leads to 3.3V, this is important for fast/slow decay!! leave at 0!

#define PWM_DIRECTION_LOW  0
#define PWM_DIRECTION_HIGH 1

int          pwm_init();
int          pwm_enable(int pwm);
int          pwm_set_period(int pwm, int period);
int          pwm_set_duty(int pwm, int duty);
int          pwm_set_polarity(int pwm, int polarity);
const char * pwm_get_ctrl_file_prefix(int pwm);
int          pwm_pull(int pwm, int direction);
int          pwm_speed(int dcPercent);

#endif // _PWMLIB_H_INCLUDED
