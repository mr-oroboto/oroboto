/**
 * main.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Simple demo to show basic use of BeagleBone Black ADC to sample MaxSonar-EZ1 and then make basic motor control
 * decisions based on the distance measured.
 */

#include <unistd.h>
#include <stdio.h>
#include "motorlib.h"
#include "pwmlib.h"
#include "adclib.h"

#define STATE_STOPPED       0
#define STATE_FORWARD       1
#define STATE_REVERSE       2
#define STATE_TURN_LEFT     3
#define STATE_TURN_RIGHT    4
#define STATE_SPIN_LEFT     5
#define STATE_SPIN_RIGHT    6

#define DISTANCE_SAFE       60
#define DISTANCE_CAUTIOUS   30

#define SPEED_FAST          50
#define SPEED_CAUTIOUS      30
#define SPEED_TURN          30

#define TIME_SAFE           1000
#define TIME_CAUTIOUS       500

/**
 * Determine the next state for the FSM.
 *
 * @param int   current_state         the FSM's current state
 * @param int   last_state            the FSM's previous state
 * @param int   current_state_cycles  the number of cycles the FSM has been in the current state
 * @param int & step_duration_msec    the number of mS the FSM should execute the next state
 *
 * @return int next_state
 */
int fsm_get_next_state(int current_state, int last_state, int current_state_cycles, int &step_duration_msec)
{
    int range = adc_sample(4, 32);           // sample ADC to determine obstacle distance

    // By default, run the step for 500ms
    step_duration_msec = TIME_SAFE;

    switch (current_state)
    {
        case STATE_STOPPED:
            printf("STATE: stopped -> forward\n");

            current_state = STATE_FORWARD;
            bot_forward(pwm_speed(SPEED_FAST));
            break;

        case STATE_FORWARD:
            if (range >= DISTANCE_SAFE)
            {
                // Nothing ahead, go full speed
                printf("STATE: forward -> forward (safe)\n");
                bot_forward(pwm_speed(SPEED_FAST));
            }
            else if (range >= DISTANCE_CAUTIOUS)
            {
                // Something ahead, go slowly
                printf("STATE: forward -> forward (cautious)\n");
                bot_forward(pwm_speed(SPEED_CAUTIOUS));
            }
            else
            {
                // Something too close, back off
                printf("STATE: forward -> reverse (cautious)\n");

                current_state = STATE_REVERSE;
                bot_reverse(pwm_speed(SPEED_FAST));
                step_duration_msec = TIME_SAFE;
            }
            break;

        case STATE_REVERSE:
            // Backing off, try spinning left for progress
            printf("STATE: reverse -> spin_left (cautious)\n");

            current_state = STATE_SPIN_LEFT;
            bot_spin(pwm_speed(SPEED_CAUTIOUS), SPIN_DIRECTION_LEFT);
            step_duration_msec = TIME_CAUTIOUS;
            break;

        case STATE_SPIN_LEFT:
            if (range <= DISTANCE_CAUTIOUS)
            {
                // Still within range of obstacle, keep spinning left
                printf("STATE: spin_left -> spin_left (cautious)\n");

                bot_spin(pwm_speed(SPEED_CAUTIOUS), SPIN_DIRECTION_LEFT);
                step_duration_msec = TIME_CAUTIOUS;
            }
            else
            {
                // Out of range of obstacle, start going forward again (slowly)
                printf("STATE: spin_left -> forward (cautious)\n");

                current_state = STATE_FORWARD;
                bot_forward(pwm_speed(SPEED_CAUTIOUS));
                step_duration_msec = TIME_CAUTIOUS;
            }
            break;
    }

    return current_state;
}

/**
 * A simple FSM to drive the bot based on the distance to any obstacle in front of us.
 */
void fsm()
{
    int current_state = STATE_STOPPED;
    int new_state;
    int last_state = STATE_STOPPED;
    int current_state_cycles = 0;
    int step_duration_msec = 0;
    int total_steps = 0;

    while (total_steps < 10000)
    {
        new_state = fsm_get_next_state(current_state, last_state, current_state_cycles, step_duration_msec);

        // Keep track of how long we've been in the current state in case the FSM wants to make decisions on that
        if (new_state != current_state)
        {
            current_state_cycles = 0;
        }
        else
        {
            current_state_cycles++;
        }

        last_state = current_state;
        current_state = new_state;

        usleep(step_duration_msec * 1000);

        total_steps++;
    }

    bot_stop();
}

/**
 * Broom, broom.
 */
int main(int argc, char *argv[])
{
    motor_init();
    adc_init();

	bot_stop();
	fsm();

	return 0;
}
