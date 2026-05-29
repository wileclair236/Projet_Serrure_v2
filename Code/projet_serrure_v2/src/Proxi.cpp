#include "Proxi.h"
void writeRegister(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(VCNL36825T_ADDR);
  Wire.write(reg);
  Wire.write(value & 0xFF);        // LSB
  Wire.write((value >> 8) & 0xFF); // MSB
  Wire.endTransmission();
}

uint16_t readRegister(uint8_t reg) {
  Wire.beginTransmission(VCNL36825T_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(VCNL36825T_ADDR, 2);
  uint16_t value = Wire.read();
  value |= (Wire.read() << 8);
  return value;
}
void Setup_proxi(int sda, int scl)
{
    Wire.begin( sda, scl,100000); // SDA, SCL (ESP32: 21, 22 par défaut)
    // Configuration de base (activer capteur)
    Serial.println("reset du capteur");
    writeRegister(0x00, 0x0001); // PS_CONF1 (exemple)
    writeRegister(0x03, 0x0000); // PS_CONF2
    writeRegister(0x04, 0x0000); // PS_CONF3
    // Serial.println("configuration du capteur");
    // writeRegister(0x00, 0x0083); // PS_ON, PS_CAL //0x0283
    // writeRegister(0x03, 0x50c0); // PS_IT,PS_Period,PS_MPS
    // writeRegister(0x04, 0x0300); // PS_HD, PS_TRIG
    Serial.println("VCNL36825T Long Range Init");

    // ------------------------------------------------
    // PS_CONF1 / PS_CONF2
    //
    // 0x0283 :
    // - PS_ON
    // - haute résolution
    // - IT plus long
    // ------------------------------------------------
    writeRegister(0x00, 0x0283);

    // ------------------------------------------------
    // PS_CONF3
    //
    // 0x71C4 :
    // - Multi pulse élevé
    // - smart persistence
    // - meilleure sensibilité
    // ------------------------------------------------
    writeRegister(0x03, 0x71C4);

    // ------------------------------------------------
    // PS_MS
    //
    // 0x0700 :
    // - LED current élevé
    // ------------------------------------------------
    writeRegister(0x04, 0x0700);

    delay(50);

    Serial.println("VCNL36825T prêt !");
}

uint16_t readProximity() {
    return readRegister(0xF8); // PS_DATA
}