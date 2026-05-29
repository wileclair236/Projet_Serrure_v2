#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"

// Définition des pins pour la caméra (OV2640)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM GPIO_NUM_15
#define SIOD_GPIO_NUM GPIO_NUM_4
#define SIOC_GPIO_NUM GPIO_NUM_5

#define Y2_GPIO_NUM GPIO_NUM_11
#define Y3_GPIO_NUM GPIO_NUM_9
#define Y4_GPIO_NUM GPIO_NUM_8
#define Y5_GPIO_NUM GPIO_NUM_10
#define Y6_GPIO_NUM GPIO_NUM_12
#define Y7_GPIO_NUM GPIO_NUM_18
#define Y8_GPIO_NUM GPIO_NUM_17
#define Y9_GPIO_NUM GPIO_NUM_16

#define VSYNC_GPIO_NUM GPIO_NUM_6
#define HREF_GPIO_NUM GPIO_NUM_7
#define PCLK_GPIO_NUM GPIO_NUM_13

extern camera_config_t config;

void camera_setup();
void capture_picture();

#endif