#ifndef Proxi_h
#define Proxi_h

#include <Arduino.h>
#include <Wire.h>

#define VCNL36825T_ADDR 0x60 // Adresse I2C du capteur VCNL36825T

void writeRegister(uint8_t reg, uint16_t value);
uint16_t readRegister(uint8_t reg);
void Setup_proxi(int sda, int scl);
uint16_t readProximity();

#endif // Proxi_h   