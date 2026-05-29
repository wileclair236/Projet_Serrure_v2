#include "Secret.h"
#include "ArduinoJson.h"
// #include "carte_sd.h"  // Assuming sd.h has SD functions, but may need to add read function

// Definitions of global variables
User users_DeJson[10];
int userCount = 0;

// Variables config
String ssid;
String wifiPassword;
String hostname;
int fuseau;

String mqttServer;
int mqttPort;
String mqttUser;
String mqttPassword;
String mqttClientID;
int mqttMaxPacket;

// Assuming SD_MMC is used, and we need to read file
// You may need to add a readFile function in sd.cpp if not present

bool loadSecretsFromSPIFFS(const char* filename) {
  File file = SPIFFS.open(filename);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }

  // Read the file content
  String jsonString = "";
  while (file.available()) {
    jsonString += (char)file.read();
  }
  file.close();

  // Parse JSON
  DynamicJsonDocument doc(1024);  // Adjust size as needed
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return false;
  }
  Serial.println("Secrets loaded from SD successfully!");
  // Extract values
  // WiFi params
  JsonArray wifiArray = doc["Parametres_WiFi"];
  if (wifiArray.size() > 0) {
    JsonObject wifi = wifiArray[0];
    ssid = wifi["SSID"] | "";
    wifiPassword = wifi["Password"] | "";
  }

  hostname = doc["Hostname"] | "";
  fuseau = doc["Fuseau_Horaire"] | 0;

  // MQTT params
  JsonArray mqttArray = doc["Parametres_MQTT"];
  if (mqttArray.size() > 0) {
    JsonObject mqtt = mqttArray[0];
    mqttServer = mqtt["Server"] | "";
    mqttPort = mqtt["Port"] | 1883;
    mqttUser = mqtt["User"] | "";
    mqttPassword = mqtt["Password"] | "";
    mqttClientID = mqtt["ClientID"] | "";
    mqttMaxPacket = mqtt["MaxPacketSize"] | 256;
  }

  Serial.println("=== Secrets Loaded ===");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("WiFi Password: "); Serial.println(wifiPassword);
  Serial.print("Hostname: "); Serial.println(hostname);
  Serial.print("Fuseau: "); Serial.println(fuseau);
  Serial.print("MQTT Server: "); Serial.println(mqttServer);
  Serial.print("MQTT Port: "); Serial.println(mqttPort);
  Serial.print("MQTT User: "); Serial.println(mqttUser);
  Serial.print("MQTT Password: "); Serial.println(mqttPassword);
  Serial.print("MQTT ClientID: "); Serial.println(mqttClientID);
  Serial.print("MQTT Max Packet: "); Serial.println(mqttMaxPacket);

  // For users array
  JsonArray usersArray = doc["users"];
  userCount = 0;
  for (JsonObject userObj : usersArray) {
    if (userCount < 8) {  // Ensure we don't exceed array bounds
      users_DeJson[userCount].uid = userObj["uid"];
      users_DeJson[userCount].fid = userObj["fid"];
      users_DeJson[userCount].name = userObj["name"] | "";
      users_DeJson[userCount].status = userObj["status"];
      Serial.printf("User %d: UID=%d, FID=%d, Name=%s, Status=%d\n", userCount, users_DeJson[userCount].uid, users_DeJson[userCount].fid, users_DeJson[userCount].name.c_str(), users_DeJson[userCount].status);
      userCount++;
    }
  }
  Serial.print("Total Users: "); Serial.println(userCount);
  Serial.println("=====================");

  return true;
}