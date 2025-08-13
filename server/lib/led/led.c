#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <softPwm.h>

#include "interface.h"

static pthread_t led_thread;
static int led_running = 0;
static int led_duty = 0;
static int led_thread_started = 0;
static pthread_mutex_t led_mutex = PTHREAD_MUTEX_INITIALIZER;

void* led_control_thread(void* arg)
{
    (void)arg;
    int prev_duty = -1;

    while (1)
    {
        pthread_mutex_lock(&led_mutex);
        if (!led_running)
        {
            pthread_mutex_unlock(&led_mutex);
            break;
        }

        int duty = led_duty;
        pthread_mutex_unlock(&led_mutex);

        if (duty != prev_duty)
        {
            softPwmWrite(LED_PIN, duty);
            prev_duty = duty;
        }

        usleep(100000);
    }
	
	softPwmWrite(LED_PIN, 0);

    pthread_mutex_lock(&led_mutex);
    led_thread_started = 0;
    pthread_mutex_unlock(&led_mutex);

    return NULL;
}

int device_init(void)
{
    pinMode(LED_PIN, OUTPUT);
    if (softPwmCreate(LED_PIN, 0, 100) != 0)
    {
        return -1;
    }
    return 0;
}

int device_control(int cmd, void *param)
{
    pthread_mutex_lock(&led_mutex);

    switch (cmd)
    {
        case LED_ON:
            led_duty = 100;
            led_running = 1;
            break;

        case LED_OFF:
            led_duty = 0;
            led_running = 0;
            break;

        case LED_PWM:
			if (!param)
			{
				pthread_mutex_unlock(&led_mutex);
				return -1;
			}

			{
				int level = *((int *)param);
				if (level < 0 || level > 2)
				{
					free(param);
					pthread_mutex_unlock(&led_mutex);
					return -1;
				}

				led_duty = (level + 1) * 25;
				led_running = 1;
				free(param);
			}
			break;

        default:
            pthread_mutex_unlock(&led_mutex);
            return -1;
    }

    if (!led_thread_started && led_running)
    {
        if (pthread_create(&led_thread, NULL, led_control_thread, NULL) == 0)
        {
            pthread_detach(led_thread);
            led_thread_started = 1;
        }
        else
        {
            printf("Failed to create LED thread\n");
            led_running = 0;
			led_thread_started = 0;
        }
    }

    pthread_mutex_unlock(&led_mutex);
    return 0;
}

void device_cleanup(void)
{
    pthread_mutex_lock(&led_mutex);
    led_running = 0;
    pthread_mutex_unlock(&led_mutex);

    softPwmWrite(LED_PIN, 0);
    softPwmStop(LED_PIN);
    pinMode(LED_PIN, INPUT);
	
	pthread_mutex_lock(&led_mutex);
    led_thread_started = 0;
    pthread_mutex_unlock(&led_mutex);
}
