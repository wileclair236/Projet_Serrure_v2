#include "timer.h"
#include <Arduino.h>
#include <Arduino.h>

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int count = 0;
bool timerFlag = false;

// Fonction appelée par l'interruption
void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  timerFlag = true;
  count++;
  if(count >= 50 ){count=0;}
  portEXIT_CRITICAL_ISR(&timerMux);
}