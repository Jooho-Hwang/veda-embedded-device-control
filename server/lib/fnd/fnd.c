#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>

#include "interface.h"

static int fnd_busy = 0;
static pthread_mutex_t fnd_mutex = PTHREAD_MUTEX_INITIALIZER;

static const int fnd_pins[] = {FND_A, FND_B, FND_C, FND_D};
static const int NUM_FND_PINS = sizeof(fnd_pins) / sizeof(fnd_pins[0]);

static void set_digit(int num)
{
    if (num < 0 || num > 9) return;

    for (int i = 0; i < NUM_FND_PINS; ++i)
    {
        int bit = (num >> i) & 0x1;
        digitalWrite(fnd_pins[i], bit);
    }
}

void* countdown_thread(void* arg)
{
    int count = *((int*)arg);

    for (int i = count; i >= 0; --i)
    {
        set_digit(i);
        sleep(1);
    }

    pthread_mutex_lock(&fnd_mutex);
    fnd_busy = 0;
    pthread_mutex_unlock(&fnd_mutex);

    return NULL;
}

int device_init(void)
{
    for (int i = 0; i < NUM_FND_PINS; ++i)
    {
        pinMode(fnd_pins[i], OUTPUT);
        digitalWrite(fnd_pins[i], LOW);
    }

    return 0;
}

int device_control(int cmd, void* param)
{
    if (cmd != FND_COUNTDOWN || param == NULL)
        return -1;

    pthread_mutex_lock(&fnd_mutex);
    if (fnd_busy)
    {
        pthread_mutex_unlock(&fnd_mutex);
        return 0;
    }
    fnd_busy = 1;
    pthread_mutex_unlock(&fnd_mutex);

    int* count_ptr = malloc(sizeof(int));
    if (!count_ptr)
    {
        pthread_mutex_lock(&fnd_mutex);
        fnd_busy = 0;
        pthread_mutex_unlock(&fnd_mutex);
        return -1;
    }

    *count_ptr = *((int*)param);

    pthread_t tid;
    if (pthread_create(&tid, NULL, countdown_thread, count_ptr) != 0)
    {
        free(count_ptr);
        pthread_mutex_lock(&fnd_mutex);
        fnd_busy = 0;
        pthread_mutex_unlock(&fnd_mutex);
        return -1;
    }

    pthread_detach(tid);
    return 0;
}

void device_cleanup(void)
{
    for (int i = 0; i < NUM_FND_PINS; ++i)
    {
        digitalWrite(fnd_pins[i], LOW);
        pinMode(fnd_pins[i], INPUT);
    }
}