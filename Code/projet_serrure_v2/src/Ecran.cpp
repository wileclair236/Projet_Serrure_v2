#include "Ecran.h"

OledSsd1315 display(Wire);
oled::OledConfig cfg;

void setup_ecran(int sda, int scl)
{
    Wire.begin(sda, scl); // adapte selon ton ESP32-S3

    cfg.i2cAddr7 = 0x3C;
    cfg.width = 128;
    cfg.height = 64;
    cfg.vccMode = VccMode::InternalChargePump;
    cfg.flip180 = true; // Rotation à 180 degrés pour une meilleure lisibilité
    display.begin(cfg);
    display.clear();
    if (display.isReady())
    {
        Serial.println("Écran OLED prêt !");
        display.fill(true);
        display.flush();
        delay(500);
        display.clear();
    }
    else
    {
        Serial.println("Erreur d'initialisation de l'écran OLED");
    }
}
void vider_ecran()
{
    if (display.isReady())
    {
        display.clear();
        display.flush();
    }
}

void drawOpenLock()
{
    if (!display.isReady())
    {
        return;
    }
    display.clear();
    display.rectFill(40, 0, 41, 2, true);
    display.rectFill(40, 2, 2, 17, true);
    display.rectFill(79, 2, 2, 17, true);
    display.rectFill(42, 17, 37, 2, true);
    display.rectFill(43, 19, 3, 17, true);
    display.rectFill(50, 19, 3, 15, true);
    display.rectFill(72, 28, 7, 2, true);
    display.rectFill(72, 30, 2, 4, true);
    display.rectFill(77, 30, 2, 8, true);
    display.rectFill(44, 36, 3, 2, true);
    display.rectFill(46, 38, 3, 2, true);
    display.rectFill(75, 38, 3, 2, true);
    display.rectFill(49, 39, 26, 2, true);
    display.rect(40, 0, 41, 2, true);
    display.rect(40, 2, 2, 17, true);
    display.rect(79, 2, 2, 17, true);
    display.rect(42, 17, 39, 2, true);
    display.rect(43, 19, 3, 17, true);
    display.rect(50, 19, 3, 15, true);
    display.rect(72, 28, 7, 2, true);
    display.rect(72, 30, 2, 4, true);
    display.rect(77, 30, 2, 8, true);
    display.rect(44, 36, 3, 2, true);
    display.rect(76, 36, 3, 2, true);
    display.rect(46, 38, 3, 2, true);
    display.rect(75, 38, 3, 2, true);
    display.pixel(45, 38, true);
    display.pixel(47, 37, true);
    display.pixel(47, 40, true);
    display.pixel(48, 40, true);
    display.pixel(49, 39, true);
    display.pixel(49, 40, true);
    display.pixel(50, 39, true);
    display.pixel(50, 40, true);
    display.pixel(51, 34, true);
    display.pixel(51, 39, true);
    display.pixel(51, 40, true);
    display.pixel(52, 34, true);
    display.pixel(52, 35, true);
    display.pixel(52, 39, true);
    display.pixel(52, 40, true);
    display.pixel(53, 34, true);
    display.pixel(53, 35, true);
    display.pixel(53, 39, true);
    display.pixel(53, 40, true);
    display.pixel(54, 34, true);
    display.pixel(54, 35, true);
    display.pixel(54, 39, true);
    display.pixel(54, 40, true);
    display.pixel(55, 34, true);
    display.pixel(55, 35, true);
    display.pixel(55, 39, true);
    display.pixel(55, 40, true);
    display.pixel(56, 34, true);
    display.pixel(56, 35, true);
    display.pixel(56, 39, true);
    display.pixel(56, 40, true);
    display.pixel(57, 34, true);
    display.pixel(57, 35, true);
    display.pixel(57, 39, true);
    display.pixel(57, 40, true);
    display.pixel(58, 34, true);
    display.pixel(58, 35, true);
    display.pixel(58, 39, true);
    display.pixel(58, 40, true);
    display.pixel(59, 34, true);
    display.pixel(59, 35, true);
    display.pixel(59, 39, true);
    display.pixel(59, 40, true);
    display.pixel(60, 34, true);
    display.pixel(60, 35, true);
    display.pixel(60, 39, true);
    display.pixel(60, 40, true);
    display.pixel(61, 34, true);
    display.pixel(61, 35, true);
    display.pixel(61, 39, true);
    display.pixel(61, 40, true);
    display.pixel(62, 34, true);
    display.pixel(62, 35, true);
    display.pixel(62, 39, true);
    display.pixel(62, 40, true);
    display.pixel(63, 34, true);
    display.pixel(63, 35, true);
    display.pixel(63, 39, true);
    display.pixel(63, 40, true);
    display.pixel(64, 34, true);
    display.pixel(64, 35, true);
    display.pixel(64, 39, true);
    display.pixel(64, 40, true);
    display.pixel(65, 34, true);
    display.pixel(65, 35, true);
    display.pixel(65, 39, true);
    display.pixel(65, 40, true);
    display.pixel(66, 34, true);
    display.pixel(66, 35, true);
    display.pixel(66, 39, true);
    display.pixel(66, 40, true);
    display.pixel(67, 34, true);
    display.pixel(67, 35, true);
    display.pixel(67, 39, true);
    display.pixel(67, 40, true);
    display.pixel(68, 34, true);
    display.pixel(68, 35, true);
    display.pixel(68, 39, true);
    display.pixel(68, 40, true);
    display.pixel(69, 34, true);
    display.pixel(69, 35, true);
    display.pixel(69, 39, true);
    display.pixel(69, 40, true);
    display.pixel(70, 34, true);
    display.pixel(70, 35, true);
    display.pixel(70, 39, true);
    display.pixel(70, 40, true);
    display.pixel(71, 34, true);
    display.pixel(71, 35, true);
    display.pixel(71, 39, true);
    display.pixel(71, 40, true);
    display.pixel(72, 34, true);
    display.pixel(72, 39, true);
    display.pixel(72, 40, true);
    display.pixel(73, 39, true);
    display.pixel(73, 40, true);
    display.pixel(74, 39, true);
    display.pixel(74, 40, true);
    display.pixel(75, 40, true);
    display.pixel(76, 40, true);
    display.flush();
}
void drawClosedLock()
{   
    if (!display.isReady())
    {
        return;
    }
    display.clear();
display.rectFill(40, 0, 41, 2, true);
display.rectFill(40, 2, 2, 17, true);
display.rectFill(79, 2, 2, 17, true);
display.rectFill(42, 17, 37, 2, true);
display.rectFill(43, 19, 3, 17, true);
display.rectFill(50, 19, 3, 15, true);
display.rectFill(72, 19, 2, 15, true);
display.rectFill(77, 19, 2, 19, true);
display.rectFill(44, 36, 3, 2, true);
display.rectFill(46, 38, 3, 2, true);
display.rectFill(75, 38, 3, 2, true);
display.rectFill(49, 39, 26, 2, true);
display.rect(40, 0, 41, 2, true);
display.rect(40, 2, 2, 17, true);
display.rect(79, 2, 2, 17, true);
display.rect(42, 17, 39, 2, true);
display.rect(43, 19, 3, 17, true);
display.rect(50, 19, 3, 15, true);
display.rect(72, 19, 2, 15, true);
display.rect(77, 19, 2, 19, true);
display.rect(44, 36, 3, 2, true);
display.rect(76, 36, 3, 2, true);
display.rect(46, 38, 3, 2, true);
display.rect(75, 38, 3, 2, true);
display.pixel(45, 38, true);
display.pixel(47, 37, true);
display.pixel(47, 40, true);
display.pixel(48, 40, true);
display.pixel(49, 39, true);
display.pixel(49, 40, true);
display.pixel(50, 39, true);
display.pixel(50, 40, true);
display.pixel(51, 34, true);
display.pixel(51, 39, true);
display.pixel(51, 40, true);
display.pixel(52, 34, true);
display.pixel(52, 35, true);
display.pixel(52, 39, true);
display.pixel(52, 40, true);
display.pixel(53, 34, true);
display.pixel(53, 35, true);
display.pixel(53, 39, true);
display.pixel(53, 40, true);
display.pixel(54, 34, true);
display.pixel(54, 35, true);
display.pixel(54, 39, true);
display.pixel(54, 40, true);
display.pixel(55, 34, true);
display.pixel(55, 35, true);
display.pixel(55, 39, true);
display.pixel(55, 40, true);
display.pixel(56, 34, true);
display.pixel(56, 35, true);
display.pixel(56, 39, true);
display.pixel(56, 40, true);
display.pixel(57, 34, true);
display.pixel(57, 35, true);
display.pixel(57, 39, true);
display.pixel(57, 40, true);
display.pixel(58, 34, true);
display.pixel(58, 35, true);
display.pixel(58, 39, true);
display.pixel(58, 40, true);
display.pixel(59, 34, true);
display.pixel(59, 35, true);
display.pixel(59, 39, true);
display.pixel(59, 40, true);
display.pixel(60, 34, true);
display.pixel(60, 35, true);
display.pixel(60, 39, true);
display.pixel(60, 40, true);
display.pixel(61, 34, true);
display.pixel(61, 35, true);
display.pixel(61, 39, true);
display.pixel(61, 40, true);
display.pixel(62, 34, true);
display.pixel(62, 35, true);
display.pixel(62, 39, true);
display.pixel(62, 40, true);
display.pixel(63, 34, true);
display.pixel(63, 35, true);
display.pixel(63, 39, true);
display.pixel(63, 40, true);
display.pixel(64, 34, true);
display.pixel(64, 35, true);
display.pixel(64, 39, true);
display.pixel(64, 40, true);
display.pixel(65, 34, true);
display.pixel(65, 35, true);
display.pixel(65, 39, true);
display.pixel(65, 40, true);
display.pixel(66, 34, true);
display.pixel(66, 35, true);
display.pixel(66, 39, true);
display.pixel(66, 40, true);
display.pixel(67, 34, true);
display.pixel(67, 35, true);
display.pixel(67, 39, true);
display.pixel(67, 40, true);
display.pixel(68, 34, true);
display.pixel(68, 35, true);
display.pixel(68, 39, true);
display.pixel(68, 40, true);
display.pixel(69, 34, true);
display.pixel(69, 35, true);
display.pixel(69, 39, true);
display.pixel(69, 40, true);
display.pixel(70, 34, true);
display.pixel(70, 35, true);
display.pixel(70, 39, true);
display.pixel(70, 40, true);
display.pixel(71, 34, true);
display.pixel(71, 35, true);
display.pixel(71, 39, true);
display.pixel(71, 40, true);
display.pixel(72, 34, true);
display.pixel(72, 39, true);
display.pixel(72, 40, true);
display.pixel(73, 39, true);
display.pixel(73, 40, true);
display.pixel(74, 39, true);
display.pixel(74, 40, true);
display.pixel(75, 40, true);
display.pixel(76, 40, true);
display.flush();
}