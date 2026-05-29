#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include <esp32-hal-timer.h>
#include <freertos/portmacro.h>
#include <esp_attr.h>

extern hw_timer_t *timer;
extern portMUX_TYPE timerMux;
extern volatile int count;
extern bool timerFlag;

void IRAM_ATTR onTimer();

#endif