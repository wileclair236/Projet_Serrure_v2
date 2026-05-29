#include "camera.h"
#include <Arduino.h>

camera_config_t config;

void camera_setup()
{
    Serial.println("Initialisation de la caméra...");
    // camera
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12; // 0-63 lower number means higher quality
    config.fb_count = 1;

    if (psramFound())
    {
        if (config.jpeg_quality < 10)
        {
            config.jpeg_quality = 10;
        }
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        if (config.jpeg_quality < 10)
        {
            config.jpeg_quality = 10;
        }
        config.frame_size = FRAMESIZE_QVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // Sensor settings
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);
        s->set_contrast(s, 1);
        s->set_saturation(s, 1);
        s->set_whitebal(s, 1);
        s->set_awb_gain(s, 1);
        s->set_exposure_ctrl(s, 1);
        s->set_aec2(s, 1);
        s->set_vflip(s, 0);
    }

    Serial.println("Camera setup terminé.");
}

void capture_picture()
{
    // Capture d'image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Échec de la capture de l'image");
        return;
    }

    // Traiter l'image capturée (par exemple, l'envoyer via MQTT, la sauvegarder sur la carte SD, etc.)
    Serial.println("Image capturée avec succès, taille: " + String(fb->len) + " octets");
    esp_camera_fb_return(fb);
}