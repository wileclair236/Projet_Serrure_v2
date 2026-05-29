#include "mqtt.h"
#include "Secret.h"
#include <Arduino.h>
#include <cstring>

WiFiClient espClient;
PubSubClient client(espClient);
String topic_message = "";
String message_mqtt = "";



// Callback lorsque un message MQTT arrive
void callback(char *topic, byte *payload, unsigned int length)
{
    topic_message = "";
    message_mqtt = "";
    Serial.print("Message arrivé [");
    Serial.print(topic);

    topic_message = String(topic);
    Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++)
    {
        message_mqtt += (char)payload[i];
    }

    message_mqtt.trim(); // enlève \n \r et caractères invisibles
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.write(payload[i]);
    }
    Serial.println();
}

// Reconnexion MQTT (non-blocking)
static unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds between retry attempts

void reconnect()
{
    // Si déjà connecté, ne rien faire
    if (client.connected())
        return;

    // Rate-limit reconnection attempts to avoid flooding
    unsigned long now = millis();
    if (now - lastReconnectAttempt < reconnectInterval)
        return;

    lastReconnectAttempt = now;
    Serial.print("Tentative de connexion MQTT...");

    // Tentative de connexion avec nom d’utilisateur et mot de passe
    if (client.connect(mqttClientID.c_str(), mqttUser.c_str(), mqttPassword.c_str(),"Serrure/status", 0, true, "offline"))
    {
        Serial.println("connecté");
        // S’abonner ici
        client.subscribe("Serrure/mode_verrou");
        client.publish("Serrure/status", "online", false);
        client.publish("Serrure/log", "MQTT connecté avec succès.", false);
    }
    else
    {
        Serial.print("échec, rc=");
        Serial.println(client.state());
    }
}

void Envoi_MQTT(char *topic, char *payload, boolean retain)
{
    // Maintenir la connexion MQTT
    if (client.state() != MQTT_CONNECTED)
    {
        Serial.printf("Mqtt state: %d", client.state());
        Serial.println("MQTT non connecté, tentative de reconnexion...");
        reconnect();
        Serial.println("Reconnecté au broker MQTT.");
    }
    client.loop();
    char msg[300];
    if (client.publish(topic, payload, retain))
    {
        Serial.println("[Topic]: " + String(topic) + " Publié: " + String(payload));
    }
    else
    {
        Serial.println("Échec de publication !");
    }
}

void initMQTT() {
    // Set default values if empty
    if (mqttServer.length() == 0) {
        mqttServer = "homeassistant.local";
    }
    if (mqttUser.length() == 0) {
        mqttUser = "MQTT_Camera";
    }
    if (mqttPassword.length() == 0) {
        mqttPassword = "1234";
    }
    if (mqttClientID.length() == 0) {
        mqttClientID = "Serrure_2026";
    }
    if (mqttPort == 0) {
        mqttPort = 1883;
    }
    if (mqttMaxPacket == 0) {
        mqttMaxPacket = 256;
    }
    
    Serial.println("=== MQTT Configuration ===");
    Serial.print("Server: "); Serial.println(mqttServer);
    Serial.print("Port: "); Serial.println(mqttPort);
    Serial.print("User: "); Serial.println(mqttUser);
    Serial.print("ClientID: "); Serial.println(mqttClientID);
    Serial.print("Max Packet: "); Serial.println(mqttMaxPacket);
    Serial.println("=========================");
    
    client.setServer(mqttServer.c_str(), (int)mqttPort);
    client.setCallback(callback);
    client.setBufferSize(mqttMaxPacket);
    client.subscribe("Serrure/mode_verrou");
    Serial.println("MQTT initialisé et abonné au topic Serrure/mode_verrou");
}