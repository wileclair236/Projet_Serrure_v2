#ifndef ECRAN_H
#define ECRAN_H

#include <Arduino.h>
#include <Wire.h>
#include <OledSsd1315.hpp>
#include "OledConfig.hpp"

using namespace oled;

extern OledSsd1315 display;
extern oled::OledConfig cfg;

void setup_ecran(int sda, int scl);
void vider_ecran();
void drawOpenLock();
void drawClosedLock();

#endif // ECRAN_H