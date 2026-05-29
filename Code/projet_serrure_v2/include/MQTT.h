#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h>

#define Verrou_CL "LOCKED"
#define Verrou_OP "UNLOCKED"
#define Verrouiallge "LOCKING"
#define Deverrouiallge "UNLOCKING"
#define Verrou_Bloque "JAMMED"
#define Verrou_Reset "None"
#define Verrou_Ouverture "OUVERTURE"

extern WiFiClient espClient;
extern PubSubClient client;
extern String topic_message;
extern String message_mqtt;

void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void Envoi_MQTT(char *topic, char *payload, boolean retain);
void initMQTT();

#endif