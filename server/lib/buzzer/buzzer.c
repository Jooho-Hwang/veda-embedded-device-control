#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <softTone.h>

#include "interface.h"

static pthread_t buzzer_thread;
static int buzzer_running = 0;
static int buzzer_thread_started = 0;
static pthread_mutex_t buzzer_mutex = PTHREAD_MUTEX_INITIALIZER;

void* buzzer_play_thread(void* arg)
{
    (void)arg;
    int melody[] = { NOTE_DO, NOTE_MI, NOTE_SO, NOTE_DO2 };
    int len = sizeof(melody) / sizeof(melody[0]);

    while (1)
    {
        pthread_mutex_lock(&buzzer_mutex);
        if (!buzzer_running)
        {
            pthread_mutex_unlock(&buzzer_mutex);
            break;
        }
        pthread_mutex_unlock(&buzzer_mutex);

        for (int i = 0; i < len; ++i)
        {
            pthread_mutex_lock(&buzzer_mutex);
            if (!buzzer_running)
            {
                pthread_mutex_unlock(&buzzer_mutex);
                break;
            }
            pthread_mutex_unlock(&buzzer_mutex);

            softToneWrite(BUZZER_PIN, melody[i]);
            usleep(300000);
        }

        softToneWrite(BUZZER_PIN, 0);
        usleep(200000);
    }

    pthread_mutex_lock(&buzzer_mutex);
    buzzer_thread_started = 0;
    pthread_mutex_unlock(&buzzer_mutex);

    return NULL;
}

int device_init(void)
{
    pinMode(BUZZER_PIN, OUTPUT);

    if (softToneCreate(BUZZER_PIN) != 0)
    {
        return -1;
    }

    return 0;
}

int device_control(int cmd, void* param)
{
    (void)param;
    pthread_mutex_lock(&buzzer_mutex);

    if (cmd == BUZZER_ON)
    {
        if (!buzzer_running)
        {
            buzzer_running = 1;

            if (!buzzer_thread_started)
            {
                if (pthread_create(&buzzer_thread, NULL, buzzer_play_thread, NULL) == 0)
                {
                    pthread_detach(buzzer_thread);
                    buzzer_thread_started = 1;
                }
                else
                {
                    buzzer_running = 0;
                    pthread_mutex_unlock(&buzzer_mutex);
                    return -1;
                }
            }
        }
    }
    else if (cmd == BUZZER_OFF)
    {
        buzzer_running = 0;
    }
    else
    {
        pthread_mutex_unlock(&buzzer_mutex);
        return -1;
    }

    pthread_mutex_unlock(&buzzer_mutex);
    return 0;
}

void device_cleanup(void)
{
    pthread_mutex_lock(&buzzer_mutex);
    buzzer_running = 0;
    pthread_mutex_unlock(&buzzer_mutex);

    softToneWrite(BUZZER_PIN, 0);
    pinMode(BUZZER_PIN, INPUT);
}
