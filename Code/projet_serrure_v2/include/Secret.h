#ifndef Secret_h
#define Secret_h
#include <Arduino.h>
#include "ArduinoJson.h"
#include "systeme_fichier.h"
#define Home_Assistant_token  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJlY2NmZGY5ZDEzOWI0MmZmYTkxYTU1OTcyYjU1NjhiNyIsImlhdCI6MTc3ODgyMjYxNiwiZXhwIjoyMDk0MTgyNjE2fQ.5hbmstnhZ8Vezv11sC0bViZgru532zDBPPPAEK5ZCek"
struct User {
  uint32_t uid;
  uint16_t fid;
  String name;
  int status;
};

extern User users_DeJson[10];
extern int userCount;

// Variables config
extern String ssid;
extern String wifiPassword;
extern String hostname;
extern int fuseau;

extern String mqttServer;
extern int mqttPort;
extern String mqttUser;
extern String mqttPassword;
extern String mqttClientID;
extern int mqttMaxPacket;

bool loadSecretsFromSPIFFS(const char* filename);

#endif
