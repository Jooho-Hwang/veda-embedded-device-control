#include <stdlib.h>
#include <dlfcn.h>
#include <wiringPi.h>
#include <syslog.h>

#include "config.h"
#include "device.h"

device_t devices[NUM_DEVICES] =
{
    {LED_LIB_PATH,    NULL, NULL, NULL, NULL},
    {BUZZER_LIB_PATH, NULL, NULL, NULL, NULL},
    {CDS_LIB_PATH,    NULL, NULL, NULL, NULL},
    {FND_LIB_PATH,    NULL, NULL, NULL, NULL},
};

int load_device_libs(void)
{
    for (int i = 0; i < NUM_DEVICES; ++i)
    {
        devices[i].handle = dlopen(devices[i].lib_path, RTLD_NOW);
        if (!devices[i].handle)
        {
            syslog(LOG_ERR, "Failed to load library: %s", devices[i].lib_path);
            unload_device_libs(i - 1);
            return (DLOPEN_FAILED + i);
        }

        devices[i].init = (init_func_t)dlsym(devices[i].handle, DEVICE_INIT_SYMBOL);
        if (!devices[i].init)
        {
            syslog(LOG_ERR, "Failed to load init symbol from: %s", devices[i].lib_path);
            unload_device_libs(i);
            return (DLSYM_INIT_FAILED + i);
        }

        devices[i].control = (control_func_t)dlsym(devices[i].handle, DEVICE_CONTROL_SYMBOL);
        if (!devices[i].control)
        {
            syslog(LOG_ERR, "Failed to load control symbol from: %s", devices[i].lib_path);
            unload_device_libs(i);
            return (DLSYM_CONTROL_FAILED + i);
        }

        devices[i].cleanup = (cleanup_func_t)dlsym(devices[i].handle, DEVICE_CLEANUP_SYMBOL);
        if (!devices[i].cleanup)
        {
            syslog(LOG_ERR, "Failed to load cleanup symbol from: %s", devices[i].lib_path);
            unload_device_libs(i);
            return (DLSYM_CLEANUP_FAILED + i);
        }

        syslog(LOG_INFO, "Successfully loaded device library: %s", devices[i].lib_path);
    }

    return 0;
}

int init_all_devices(void)
{
    if (wiringPiSetup() == -1)
    {
        syslog(LOG_ERR, "Failed to initialize wiringPi");
        return WIRINGPISETUP_FAILED;
    }

    for (int i = 0; i < NUM_DEVICES; ++i)
    {
        if (devices[i].init && devices[i].init() < 0)
        {
            syslog(LOG_ERR, "Failed to initialize device %d", i);
            return (DEVICE_INIT_FAILED + i);
        }
        syslog(LOG_INFO, "Device %d initialized successfully", i);
    }

    return 0;
}

void cleanup_all_devices(void)
{
    for (int i = 0; i < NUM_DEVICES; ++i)
    {
        if (devices[i].cleanup)
        {
            devices[i].cleanup();
            syslog(LOG_INFO, "Device %d cleaned up", i);
        }
    }
}

void unload_device_libs(int _up_to_index)
{
    for (int i = 0; i <= _up_to_index; ++i)
    {
        if (devices[i].handle)
        {
            dlclose(devices[i].handle);
            devices[i].handle = NULL;
            devices[i].init = NULL;
            devices[i].control = NULL;
            devices[i].cleanup = NULL;
            syslog(LOG_INFO, "Unloaded library for device %d", i);
        }
    }
}
