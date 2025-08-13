#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "interface.h"

static int i2c_fd = -1;
static pthread_t cds_thread;
static int cds_running = 0;
static int cds_thread_started = 0;
static int cds_threshold = 90;
static pthread_mutex_t cds_mutex = PTHREAD_MUTEX_INITIALIZER;

void* cds_monitor_thread(void* arg)
{
    (void)arg;
	
	struct timeval start, now;
    gettimeofday(&start, NULL);

    while (1)
    {
        pthread_mutex_lock(&cds_mutex);
        if (!cds_running)
        {
            pthread_mutex_unlock(&cds_mutex);
            break;
        }

        int threshold = cds_threshold;
        pthread_mutex_unlock(&cds_mutex);

        int val = wiringPiI2CRead(i2c_fd);
        if (val >= 0)
        {
            if (val < threshold)
                digitalWrite(CDS_LED_PIN, HIGH);
            else
                digitalWrite(CDS_LED_PIN, LOW);
        }

        usleep(250000);
		
		gettimeofday(&now, NULL);
        long elapsed_ms = (now.tv_sec - start.tv_sec) * 1000L +
                          (now.tv_usec - start.tv_usec) / 1000L;

        if (elapsed_ms >= 10000)
            break;
    }
	
	digitalWrite(CDS_LED_PIN, LOW);

    pthread_mutex_lock(&cds_mutex);
	cds_running = 0;
    cds_thread_started = 0;
    pthread_mutex_unlock(&cds_mutex);

    return NULL;
}

int device_init(void)
{
    i2c_fd = wiringPiI2CSetup(CDS_ADDR);
    if (i2c_fd < 0)
        return -1;

    pinMode(CDS_LED_PIN, OUTPUT);
    digitalWrite(CDS_LED_PIN, LOW);

    return 0;
}

int device_control(int cmd, void *param)
{
	(void)param;
	
    pthread_mutex_lock(&cds_mutex);

    if (cmd == CDS_READ)
    {
        if (!cds_running)
        {
            cds_running = 1;
            if (!cds_thread_started)
            {
                if (pthread_create(&cds_thread, NULL, cds_monitor_thread, NULL) == 0)
                {
                    pthread_detach(cds_thread);
                    cds_thread_started = 1;
                }
                else
                {
                    cds_running = 0;
                    pthread_mutex_unlock(&cds_mutex);
                    return -1;
                }
            }
        }
    }
    else
    {
        pthread_mutex_unlock(&cds_mutex);
        return -1;
    }

    pthread_mutex_unlock(&cds_mutex);
    return 0;
}

void device_cleanup(void)
{
    pthread_mutex_lock(&cds_mutex);
    cds_running = 0;
    pthread_mutex_unlock(&cds_mutex);

    digitalWrite(CDS_LED_PIN, LOW);
    pinMode(CDS_LED_PIN, INPUT);
}
