#ifndef CONFIG_H
#define CONFIG_H

// -----------------------------
// server
// -----------------------------
#define HJH_SERVER              "hjh_server"
#define GETRLMINIT_FAILED       -101
#define FORK_FAILED             -102
#define SETSID_FAILED           -103
#define SIGACTION_FAILED        -104
#define CHDIR_FAILED            -105
#define STDIN_CON_FAILED        -106
#define STDIN_DUP_FAILED        -107
#define STDOUT_CON_FAILED       -108
#define STDOUT_DUP_FAILED       -109
#define STDERR_CON_FAILED       -110
#define STDERR_DUP_FAILED       -111

// -----------------------------
// device
// -----------------------------
#define NUM_DEVICES             4

#define DEVICE_INIT_SYMBOL      "device_init"
#define DEVICE_CONTROL_SYMBOL   "device_control"
#define DEVICE_CLEANUP_SYMBOL   "device_cleanup"

#define LED_LIB_PATH            "/mnt/server/lib/led/libled.so"
#define BUZZER_LIB_PATH         "/mnt/server/lib/buzzer/libbuzzer.so"
#define CDS_LIB_PATH            "/mnt/server/lib/cds/libcds.so"
#define FND_LIB_PATH            "/mnt/server/lib/fnd/libfnd.so"

#define DLOPEN_FAILED           -120
#define DLSYM_INIT_FAILED       -130
#define DLSYM_CONTROL_FAILED    -140
#define DLSYM_CLEANUP_FAILED    -150
#define WIRINGPISETUP_FAILED    -160
#define DEVICE_INIT_FAILED      -170
#define DEVICE_CONTROL_FAILED   -180

// -----------------------------
// socket
// -----------------------------
#define PORT                    60000
#define BACKLOG                 5

#define SOCKET_FAILED           -191
#define SETSOCKETOPT_FAILED     -192
#define BIND_FAILED             -193
#define LISTEN_FAILED           -194

// -----------------------------
// handler
// -----------------------------
#define BUFF_SIZE               1024

#define ACCEPT_FAILED           -201
#define RECV_FAILED             -202
#define STRNCMP_FAILED          -203
#define STRDUP_FAILED           -204
#define CMD_NUM_OUT_OF_RANGE    -205
#define MALLOC_TASK_FAILED      -206
#define PARAM_OUT_OF_RANGE      -207
#define MALLOC_PARAM_FAILED     -208
#define PTHREAD_CREATE_FAILED   -209
#define SEND_FAILED             -210

#endif // CONFIG_H