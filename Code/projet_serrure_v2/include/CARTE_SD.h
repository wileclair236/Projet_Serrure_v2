#ifndef CARTE_SD_H
#define CARTE_SD_H

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

class CarteSD
{
public:
    bool begin(int clk, int cmd, int d0);
    void info();
    bool createDir(const char *path);
    bool exists(const char *path);
    String readFile(const char *path);
    bool writeFile(const char *path, const char *message);

private:
    bool initialized = false;
};

#endif