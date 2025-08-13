#ifndef INTERFACE_H
#define INTERFACE_H

int device_init(void);
int device_control(int cmd, void *param);
void device_cleanup(void);

// -----------------------------
// LED
// -----------------------------
#define LED_OFF               0
#define LED_ON                1
#define LED_PWM               2
#define LED_PIN              26    // wiringPi 26 → BCM GPIO 12

// -----------------------------
// BUZZER
// -----------------------------
#define BUZZER_OFF            0
#define BUZZER_ON             1
#define BUZZER_PIN           23    // wiringPi 23 → BCM GPIO 13

#define NOTE_DO             262
#define NOTE_MI             330
#define NOTE_SO             392
#define NOTE_DO2            523

// -----------------------------
// CDS
// -----------------------------
#define CDS_READ              0
#define CDS_ADDR           0x48
#define CDS_LED_PIN          15    // wiringPi 15 → BCM GPIO 14

// -----------------------------
// FND
// -----------------------------
#define FND_COUNTDOWN  0

#define FND_A                 0    // wiringPi 0 → GPIO 17
#define FND_B                 2    // wiringPi 2 → GPIO 27
#define FND_C                 3    // wiringPi 3 → GPIO 22
#define FND_D                 4    // wiringPi 4 → GPIO 23

#endif // INTERFACE_H