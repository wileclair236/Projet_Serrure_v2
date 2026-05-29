#ifndef systeme_fichier_h
#define systeme_fichier_h

#include <Arduino.h>
#include "SPIFFS.h"
#include "FS.h"
#include "Camera.h"
#include "Timer.h"
void saveImageToSPIFFS(camera_fb_t* fb);

class SystemeFichier
{
public:
    bool begin();
    void info();
    bool exists(const char *path);
    void contenu();
    String readFile(const char *path);
    bool writeFile(const char *path, const char *message);

private:
    bool initialized = false;
};
#endif