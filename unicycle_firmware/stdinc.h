#include <Arduino.h>
#include <SPI.h>
#include <ICM20948_WE.h>

#include "ram.h"

#define LOOP_MS 2
#define MS_TO_LOOP(ms) ((ms) / LOOP_MS)
#define LOOP_S 0.002f

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define LIMIT(x, min, max)  (((x) < (min)) ? (min) : (((x) < (max)) ? (x) : (max)))
#define PT1(new_val, old_val, time_constant_ms) ((old_val) + ((new_val) - (old_val)) / MS_TO_LOOP(time_constant_ms))
#define BUTTON(x) ((joystickReport.buttons & (1 << (x))) >> (x))
#define fsign(x) ((x) > 0 ? 1 : -1)

void setupWifi();
void setupOta();
void setupSensors();
void setupControl();
void setupUI();
void setupInputOutput();
void setupBLE();
void setupCan();

void loopOta();
void loopSensors();
void loopControl();
void loopUI();
void loopInputOutput();
void loopBLE();
void loopCan();

void ledColor(uint8_t r, uint8_t g, uint8_t b);
void disconnectWifi();
void disconnectBLE();
bool sendTorque(uint8_t axis, float tq);
