#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "freertos/semphr.h"
#include <Arduino.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "mqtt.h"
#include "camera.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include "systeme_fichier.h"
#include "Secret.h"

// ── Configuration Home Assistant ──────────────────────────────────────────
#define HA_IP    "homeassistant.local"      // ou IP fixe: "192.168.1.XX"
#define HA_PORT  8123
#define HA_TOKEN Home_Assistant_token // Profil → Sécurité → Long-Lived Access Tokens

/* ============ IMAGE CONFIG ============ */
#define IMG_WIDTH  320
#define IMG_HEIGHT 240
#define IMG_PIXELS (IMG_WIDTH * IMG_HEIGHT)

/* ============ MOTION PARAMS ============ */
#define PIXEL_DIFF_THRESHOLD 80
#define ZONE_COLS 5
#define ZONE_ROWS 5
#define ZONE_PIXEL_THRESHOLD 270
#define HUMAN_ZONE_COUNT 10

/* ============ GLOBALS ============ */
extern httpd_handle_t camera_httpd;
extern uint8_t *gray_curr;
extern uint8_t *gray_prev;
extern uint8_t *last_jpeg;
extern size_t last_jpeg_len;
extern SemaphoreHandle_t jpeg_mutex;
extern SemaphoreHandle_t cam_mutex;
extern bool first_frame;
extern bool humain_detecte;
extern uint32_t dernier_dection;

/* ============ FONCTIONS ============ */
void startCameraServer();

// Détection
void fb_to_gray(camera_fb_t *fb, uint8_t *gray_buf);
bool detect_human_by_zone(uint8_t *curr, uint8_t *prev);

// Tâche caméra
void camera_task(void *pvParameters);

// Home Assistant
bool sendImageToHA(const char *filepath);
void sendAllImagesToHA();

// Endpoints SPIFFS
// GET /stream                             → stream MJPEG live
// GET /image?f=/img_YYYYMMDD_HHMMSS.jpg  → servir une image précise
// GET /last                               → dernière image capturée
// GET /images                             → liste JSON de toutes les images
// GET /gallery                            → galerie HTML des 20 dernières images

#endif