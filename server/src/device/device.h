#ifndef DEVICE_H
#define DEVICE_H

typedef int (*init_func_t)(void);
typedef int (*control_func_t)(int, void *);
typedef void (*cleanup_func_t)(void);

typedef struct
{
    char *lib_path;
    void *handle;
    init_func_t init;
    control_func_t control;
    cleanup_func_t cleanup;
} device_t;

extern device_t devices[NUM_DEVICES];

int load_device_libs(void);
int init_all_devices(void);
void cleanup_all_devices(void);
void unload_device_libs(int);

#endif // DEVICE_H