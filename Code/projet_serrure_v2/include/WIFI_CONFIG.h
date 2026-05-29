#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include "time.h"

extern struct tm timeinfo;

struct WiFiConfig {
    String ssid;
    String password;
    int fuseau_Horaire;
    String hostname;
};

extern WiFiConfig WifiConfig;

void wifi_setup(const char* ssid, const char* password, int fuseau_Horaire, const char* hostname);
String readUserInputWithBackspace1(const String& ssid);

#endif