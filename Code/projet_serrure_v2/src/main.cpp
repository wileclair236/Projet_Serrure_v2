#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "http_server.h"
#include "camera.h"
#include "img_converters.h"
#include "esp_http_server.h"
#include "freertos/semphr.h"
#include "wifi_config.h"
#include "Secret.h"
#include "mqtt.h"
#include "timer.h"
// #include "SD.h"
// #include "carte_sd.h"
#include <systeme_fichier.h>
#include <MFRC522.h>
#include <Users.h>
#include "fpm.h"
#include "fpm_logging.h"
#include "ArduinoJson.h"
#include "Proxi.h"
#include "Ecran.h"

// ✅ FIX: #include doivent être au niveau global, jamais dans une fonction
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "esp_attr.h"

enum etat_serrure
{
  Menu,
  Fonctionnement,
  Configuration,
  Sauvegarde
};
etat_serrure etat_actuel = Menu;

enum verrou
{
  barre_op,
  barre_cl,
  verrou_open,
  verrou_close
};
verrou etat_verrou = barre_cl;
#define MAX_USERS 10
RFIDTag users[MAX_USERS];

#define adc_pin 3
#define Commande_ouverture_porte_pin 14
/* ============ PWM servo PARAMS ============ */
#define PWM_servo_FREQ 50
#define PWM_servo_CHANNEL 0
#define PWM_servo_RESOLUTION 12
#define PWM_servo_PIN 21
uint32_t servo_duty_cycle = (1.5 * 4095) / 20.0;
int count_timer_porte_op;
/* ============ PWM vibreur PARAMS ============ */
#define PWM_vibreur_FREQ_autorise 4000
#define PWM_vibreur_FREQ_non_autorise 3000
#define PWM_vibreur_CHANNEL 2
#define PWM_vibreur_RESOLUTION 12
#define PWM_vibreur_PIN 46
uint32_t vibreur_duty_cycle = (125 * 4095) / 250;
/* ============ SPI PARAMS ============ */
#define RFID_SS_PIN 42
#define RST_PIN -1
#define SCK_PIN 39
#define MOSI_PIN 38
#define MISO_PIN 40
#define IRQ_RFID_PIN 41
/* ============ SD PARAMS ============ */
SystemeFichier systemeFichier;
File file;
/* ============ LECTEUR D'EMPREINTE DIGITAL PARAMS ============ */
#define TX_finger_PIN 47
#define RX_finger_PIN 48
int16_t fid2 = 0;
/* ============ PROXIMITY SENSOR PARAMS ============ */
#define SDA_PIN 1
#define SCL_PIN 2
/* ============ json document ============ */
JsonDocument doc;
JsonObject root = doc.to<JsonObject>();
/* ============ Oled ecran ============ */
using namespace oled;

/* ============ PARAMS ============ */
#define nbr_max_rep 50
#define division_rep 5
char payload[128];

//==========================================================//
void barre_ouverte(bool activation);
void barre_ferme(bool activation);
void deverrouiallge(bool activation);
bool porte_ouverture(void);
uint8_t etat_batterie(uint8_t niveau_batterie_precedent = 255);

RTC_DATA_ATTR char last_panic_hint[64] = {0};

void setup()
{
  Serial.begin(115200);
  delay(6000); // ← 6 secondes pour ouvrir le moniteur AVANT le crash suivant

  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("=== RESET CAUSE: %d ===\n", reason);

  if (reason == ESP_RST_PANIC)
  {
    Serial.printf("=== DERNIER CHECKPOINT: %s ===\n", last_panic_hint);
  }

  // Reset le hint
  strlcpy(last_panic_hint, "setup_start", sizeof(last_panic_hint));

  Serial.printf("Heap libre: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM libre: %d bytes\n", ESP.getFreePsram());

  delay(1000);
  Serial.println("Démarrage de l'ESP32...");
  delay(2000);

  // Initialisation GPIO
  pinMode(PWM_servo_PIN, OUTPUT);
  pinMode(PWM_vibreur_PIN, OUTPUT);
  pinMode(adc_pin, INPUT);
  pinMode(Commande_ouverture_porte_pin, INPUT);

  ledcSetup(PWM_servo_CHANNEL, PWM_servo_FREQ, PWM_servo_RESOLUTION);
  ledcAttachPin(PWM_servo_PIN, PWM_servo_CHANNEL);
  ledcWrite(PWM_servo_CHANNEL, servo_duty_cycle);

  ledcSetup(PWM_vibreur_CHANNEL, PWM_vibreur_FREQ_autorise, PWM_vibreur_RESOLUTION);
  ledcAttachPin(PWM_vibreur_PIN, PWM_vibreur_CHANNEL);
  ledcWrite(PWM_vibreur_CHANNEL, vibreur_duty_cycle);

  // --- Configuration du timer ---
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 200000, true); // 200ms
  // Le timer est démarré dans le mode fonctionnement, après le menu de sélection
  // --- Fin configuration du timer ---

  setupRFID(SCK_PIN, MISO_PIN, MOSI_PIN, RFID_SS_PIN, IRQ_RFID_PIN, RST_PIN);
  delay(1000);

  systemeFichier.begin();
  systemeFichier.info();
  systemeFichier.contenu();
  camera_setup();
  delay(1000);

  Serial.println("Initialisation du lecteur d'empreinte digitale...");
  finger.Lecteur_Empreinte_Digital_Setup(RX_finger_PIN, TX_finger_PIN, 45);
  finger.ledConfigure(FPMLedControl::Allume, 0x40, FPMLedColour::Cyan);
  finger.ledOn();
  Serial.println("Vérification du capteur de l'empreinte digitale...");

  Setup_proxi(SDA_PIN, SCL_PIN);
  setup_ecran(SDA_PIN, SCL_PIN);
  delay(1000);
  vider_ecran();
  capture_picture();
  capture_picture();
  Serial.println("Setup terminé.");
}

void loop()
{
  if (etat_actuel == Menu)
  {
    // Rester dans le menu
  }
  else if (etat_actuel == Configuration)
  {
    etat_actuel = Configuration;
  }
  else if (etat_actuel == Sauvegarde)
  {
    etat_actuel = Sauvegarde;
  }
  else
  {
    etat_actuel = Fonctionnement;
    timerAlarmEnable(timer);
    finger.ledConfigure(FPMLedControl::Allume, 255, FPMLedColour::Violet, 10);
    finger.ledOn();
    Serial.println("Mode fonctionnement activé.");
  }

  switch (etat_actuel)
  {
  case Menu:
  {
    Serial.println("=== MENU ===");
    Serial.println("1. Passer en mode sauvegarde");
    Serial.println("2. Passer en mode configuration");
    Serial.println("Entrez votre choix (1 ou 2) :");

    int choice = 0;
    int Count_loop = 0;
    while (choice == 0)
    {
      if (Serial.available() > 0)
      {
        choice = Serial.parseInt();
        Serial.println("Choix reçu : " + String(choice));
        if (choice == 1)
          etat_actuel = Sauvegarde;
        else if (choice == 2)
          etat_actuel = Configuration;
      }
      delay(10);
      Count_loop++;
      if (digitalRead(Commande_ouverture_porte_pin) == HIGH)
      {
        Serial.println("Bouton pressé, passage en mode configuration.");
        choice = 2;
        etat_actuel = Configuration;
        break;
      }
      else if (Count_loop > 500) // 5 secondes à 10ms de delay
      {
        Serial.println("Aucun choix reçu, passage en mode sauvegarde.");
        choice = 1;
        etat_actuel = Sauvegarde;
        break;
      }
    }
  }
  break;

  case Fonctionnement:
  {
    static int i = 1;
    int nbr_repetitions = nbr_max_rep / division_rep; // = 10
    uint32_t tag_scanner;

    // ✅ Variable de debug stack — affichage périodique seulement
    static uint32_t last_stack_print = 0;
    uint8_t pourcentage_batterie = 1;

    while (1)
    {
      // ✅ FIX: Affichage stack toutes les 5 secondes (pas toutes les 1ms)
      if (millis() - last_stack_print > 5000)
      {
        Serial.printf("Stack libre: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
        last_stack_print = millis();
      }

      bool flag = false;
      // ✅ Lecture protégée du flag timer
      portENTER_CRITICAL(&timerMux);
      flag = timerFlag;
      portEXIT_CRITICAL(&timerMux);

      if (flag)
      {
        portENTER_CRITICAL(&timerMux);
        timerFlag = false;
        portEXIT_CRITICAL(&timerMux);

        // Maintenir MQTT actif
        client.loop();

        uint16_t FID_scan;
        bool commande_ouverture = digitalRead(Commande_ouverture_porte_pin);

        // ✅ FIX: Snapshot de count avec section critique pour toutes les comparaisons
        portENTER_CRITICAL(&timerMux);
        int local_count = count;
        portEXIT_CRITICAL(&timerMux);

        if (local_count == (i * (nbr_repetitions - 1)))
        {
          Serial.println("Commande reçue via MQTT: " + message_mqtt);

          if ((topic_message == "Serrure/mode_verrou") && (message_mqtt == Verrou_OP || message_mqtt == "UNLOCK"))
          {
            barre_ouverte(1);
          }
          else if (commande_ouverture == HIGH)
          {
            Serial.println("Bouton d'ouverture de porte pressé.");
            deverrouiallge(1);
            drawOpenLock();
            finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Vert, 40);
            finger.ledOn();
            snprintf(payload, sizeof(payload), "Porte ouverte via bouton ou clé.");
            Envoi_MQTT("Serrure/log", payload, false);
            while (!porte_ouverture())
            {
            }
            vider_ecran();
            finger.ledConfigure(FPMLedControl::Allume, 255, FPMLedColour::Violet, 10);
            finger.ledOff();
          }
          else if ((tag_scanner = Detection_TagsRFID_IRQ()) > 0)
          {
            for (size_t j = 0; j < users[0].getNumConfiguredUsers(); j++)
            {
              if ((tag_scanner == users[j].getID()) && (users[j].getStatus() == WHITELIST))
              {
                Serial.printf("Utilisateur: %s -> accès autorisé\n", users[j].getUserName().c_str());
                deverrouiallge(1);
                drawOpenLock();
                finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Vert);
                finger.ledOn();
                snprintf(payload, sizeof(payload), "Utilisateur: %s -> accès autorisé", users[j].getUserName().c_str());
                Envoi_MQTT("Serrure/log", payload, false);
                snprintf(payload, sizeof(payload), "{\"title\":\"✅ Utilisateur reconnu\",\"message\":\"Utilisateur: %s -> accès autorisé\"}", users[j].getUserName().c_str());
                Envoi_MQTT("Serrure/notification", payload, true);
                while (!porte_ouverture())
                {
                }
                vider_ecran();
                finger.ledConfigure(FPMLedControl::Allume, 255, FPMLedColour::Violet, 10);
                finger.ledOff();
              }
              else if ((tag_scanner == users[j].getID()) && (users[j].getStatus() == BLACKLIST))
              {
                Envoi_MQTT("Serrure/mode_verrou", Verrouiallge, true);
                drawClosedLock();
                finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Rouge, 10);
                finger.ledOn();
                ledcWriteTone(PWM_vibreur_CHANNEL, PWM_vibreur_FREQ_non_autorise);
                Serial.printf("Utilisateur: %s -> accès non autorisé\n", users[j].getUserName().c_str());
                // ✅ FIX: Utiliser j (index courant) au lieu de FID_scan (non initialisé ici)
                snprintf(payload, sizeof(payload), "{\"title\":\"❌ Utilisateur non autorisé\",\"message\":\"Utilisateur: %s -> accès non autorisé\"}", users[j].getUserName().c_str());
                Envoi_MQTT("Serrure/notification", payload, false);
                deverrouiallge(1);
              }
              else
              {
                ledcWriteTone(PWM_vibreur_CHANNEL, PWM_vibreur_FREQ_non_autorise);
                drawClosedLock();
                finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Rouge, 10);
                finger.ledOn();
                deverrouiallge(1);
              }
            }
          }
          else if (Finger_irqFlag == true)
          {
            if (finger.Recherche_empreinte_correspondante(FID_scan))
            {
              Finger_irqFlag = false;
              Serial.printf("Empreinte digitale scannée avec FID: %d\n", FID_scan);
              Serial.printf("Utilisateur trouvé : %s\n", users[FID_scan].getUserName().c_str());
              deverrouiallge(1);
              snprintf(payload, sizeof(payload), "Utilisateur: %s -> accès autorisé", users[FID_scan].getUserName().c_str());
              Envoi_MQTT("Serrure/log", payload, true);
              snprintf(payload, sizeof(payload), "{\"title\":\"✅ Utilisateur reconnu\",\"message\":\"Utilisateur: %s -> accès autorisé\"}", users[FID_scan].getUserName().c_str());
              Envoi_MQTT("Serrure/notification", payload, true);
              drawOpenLock();
              finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Vert);
              finger.ledOn();
              while (!porte_ouverture())
              {
              }
              vider_ecran();
              finger.ledConfigure(FPMLedControl::Allume, 255, FPMLedColour::Violet, 10);
              finger.ledOff();
            }
            else
            {
              Finger_irqFlag = false;
              drawClosedLock();
              finger.ledConfigure(FPMLedControl::Clignotement_Flash, 50, FPMLedColour::Rouge, 10);
              finger.ledOn();
              ledcWriteTone(PWM_vibreur_CHANNEL, PWM_vibreur_FREQ_non_autorise);
              camera_fb_t* fb = esp_camera_fb_get();
              saveImageToSPIFFS(fb);
              Serial.println("Empreinte digitale non reconnue.");
              snprintf(payload, sizeof(payload), "Empreinte digitale non reconnue.");
              Envoi_MQTT("Serrure/log", payload, false);
              snprintf(payload, sizeof(payload), "{\"title\":\"🚨 tentative d'intrusion\",\"message\":\"Empreinte digitale non reconnue.\"}");
              Envoi_MQTT("Serrure/notification", payload, true);

              sendAllImagesToHA();
              esp_camera_fb_return(fb);
              deverrouiallge(1);
            }
          }
          else if ((topic_message == "Serrure/mode_verrou") && (message_mqtt == Verrou_Ouverture))
          {
            deverrouiallge(1);
            Envoi_MQTT("Serrure/log", "Porte ouverte via accès à distance.", false);
            drawOpenLock();
            while (!porte_ouverture())
            {
            }
          }
          else if ((topic_message == "Serrure/mode_verrou") && (message_mqtt == Verrou_CL || message_mqtt == "LOCK"))
          {
            barre_ferme(1);
          }
          else
          {

            Serial.println("Commande inconnue.");
            Serial.println("Message MQTT: " + topic_message + " -> " + message_mqtt);
            Envoi_MQTT("Serrure/mode_verrou", Verrou_CL, true);
          }

          // Envoi de l'heure via MQT
          if (local_count == (1 || 5 || 9 || 13 || 17 || 21 || 25 || 29 || 33 || 37)) // = toutes les 4 secondes (200ms * 20)
          {
            Serial.println("Envoi de l'heure actuelle via MQTT...");
            getLocalTime(&timeinfo);
            strftime(payload, sizeof(payload), "%d/%m/%Y %Hh%Mm%Ss", &timeinfo);
            Envoi_MQTT("Serrure/message_time/home_Assistant", payload, true);
          }

          // ✅ FIX: Utiliser local_count pour la comparaison (déjà snapshottée)
          if (local_count == 36)
          {
            pourcentage_batterie = etat_batterie(pourcentage_batterie);
            snprintf(payload, sizeof(payload), "{\"Pourcentage_Batterie\":%d}", etat_batterie(pourcentage_batterie));
            Envoi_MQTT("Serrure/capteur/Batterie", payload, true);
            snprintf(payload, sizeof(payload), "{\"Proximite\":%d}", readProximity());
            Envoi_MQTT("Serrure/capteur/proximite", payload, true);
          }

          i = (i < division_rep) ? i + 1 : 1;
        }

        // ✅ FIX: Utiliser local_count pour la comparaison de dépassement
        if (local_count > nbr_max_rep)
        {
          Serial.printf("count: %d dépasse seuil, réinitialisation.\n", local_count);
          portENTER_CRITICAL(&timerMux);
          count = 0;
          portEXIT_CRITICAL(&timerMux);
          i = 1; // i est local à cette tâche, pas besoin de section critique
        }
      }

      delay(1);
    }
  }
  break;

  case Configuration:
  {
    wifi_setup("", "", 0xff, "");
    char dateStr_json[32];
    getLocalTime(&timeinfo);
    strftime(dateStr_json, sizeof(dateStr_json), "%d/%m/%Y %Hh%Mm%Ss", &timeinfo);
    root["date_creation"] = dateStr_json;

    JsonArray tableau_wifi = root.createNestedArray("Parametres_WiFi");
    Serial.println("Paramètres WiFi ajoutés au JSON.");
    Serial.printf("SSID: %s, Password: %s, Hostname: %s, Fuseau_Horaire: %d\n",
                  WifiConfig.ssid.c_str(), WifiConfig.password.c_str(),
                  WifiConfig.hostname.c_str(), WifiConfig.fuseau_Horaire);
    JsonObject wifi_json = tableau_wifi.createNestedObject();
    wifi_json["SSID"] = WifiConfig.ssid;
    wifi_json["Password"] = WifiConfig.password;
    root["Hostname"] = WifiConfig.hostname;
    root["Fuseau_Horaire"] = WifiConfig.fuseau_Horaire;

    initMQTT();
    JsonArray tableau_MQTT = root.createNestedArray("Parametres_MQTT");
    JsonObject mqtt_json = tableau_MQTT.createNestedObject();
    mqtt_json["Server"] = mqttServer;
    mqtt_json["Port"] = mqttPort;
    mqtt_json["User"] = mqttUser;
    mqtt_json["Password"] = mqttPassword;
    mqtt_json["ClientID"] = mqttClientID;
    mqtt_json["MaxPacketSize"] = mqttMaxPacket;

    finger.emptyDatabase();

    Serial.println("=== MODE CONFIGURATION ===");
    Serial.print("Combien de users RFID voulez-vous configurer ? (1 à " + String(MAX_USERS) + ") : ");

    // ✅ FIX: Ne pas réutiliser count (variable partagée avec l'ISR) pour autre chose
    // Utiliser une variable locale dédiée
    int nb_users_config = 0;
    while ((nb_users_config == 0) || (nb_users_config > MAX_USERS))
    {
      while (Serial.available() == 0)
      {
      }
      nb_users_config = Serial.parseInt();
      if (nb_users_config < 1 || nb_users_config > MAX_USERS)
      {
        Serial.println("Veuillez entrer un nombre entre 1 et " + String(MAX_USERS) + ".");
        nb_users_config = 0;
      }
      else
      {
        Serial.println(nb_users_config);
      }
    }

    delay(1000);
    Serial.println("Veuillez présenter user");

    for (int i = 0; i < nb_users_config; i++)
    {
      while (users[i].getID() == 0)
      {
        if (uint32_t tag_uid = Detection_TagsRFID_IRQ())
        {
          delay(100);
          finger.getFreeId(&fid2);
          Serial.printf("ID libre trouvé : %d\n", fid2);
          Serial.printf("Veuillez présenter l'empreinte digitale de l'utilisateur %d...\n", i);
          delay(1000);
          while (!finger.Ajout_empreinte_digitale(fid2))
          {
            delay(100);
          }
          users[i] = RFIDTag(tag_uid, fid2, readUserInputWithBackspace(), readStatus());
        }
        else
        {
          Serial.println("En attente de détection du tag RFID...");
        }
      }
      Serial.println("User configuré :");
      users[i].printInfo();
      snprintf(payload, sizeof(payload), "User configuré : %s", users[i].getUserName().c_str());
      Envoi_MQTT("Serrure/log", payload, false);
    }
    delay(1000);

    JsonArray arr = root.createNestedArray("users");
    for (int i = 0; i < users[0].getNumConfiguredUsers(); i++)
    {
      JsonObject users_json = arr.createNestedObject();
      users_json["uid"] = users[i].getID();
      users_json["fid"] = users[i].getIdFinger();
      users_json["name"] = users[i].getUserName();
      users_json["status"] = users[i].getStatus();
    }

    serializeJsonPretty(doc, file);
    if (systemeFichier.writeFile("/config.json", doc.as<String>().c_str()))
    {
      Serial.println("JSON écrit sur SPIFFS !");
      Serial.println("Contenu du JSON :");
      Serial.print(systemeFichier.readFile("/config.json"));
      Serial.println("");
      file.close();
    }
    else
    {
      Serial.println("Erreur ouverture fichier !");
    }
    delay(2000);
    startCameraServer();
    Envoi_MQTT("Serrure/log", "Setup serrure terminé", false);
    etat_actuel = Fonctionnement;
  }
  break;

  case Sauvegarde:
  {
    if (!loadSecretsFromSPIFFS("/config.json"))
    {
      Serial.println("Failed to load secrets from SPIFFS");
      etat_actuel = Configuration;
    }
    else
    {
      wifi_setup(ssid.c_str(), wifiPassword.c_str(), fuseau, hostname.c_str());
      initMQTT();
      for (int i = 0; i < userCount; i++)
      {
        users[i] = RFIDTag(users_DeJson[i].uid, users_DeJson[i].fid, users_DeJson[i].name, users_DeJson[i].status);
        Serial.printf("uid: %d, fid: %d, name: %s, status: %d\n",
                      users[i].getID(), users[i].getIdFinger(),
                      users[i].getUserName().c_str(), users[i].getStatus());
      }
      startCameraServer();
      Envoi_MQTT("Serrure/log", "Setup serrure terminé", false);
      etat_actuel = Fonctionnement;
    }
  }
  break; // ✅ FIX: break manquant — évite le fall-through vers default

  default:
    Serial.println("État inconnu.");
    break;
  }
}

void barre_ouverte(bool activation)
{
  if (activation)
  {
    Envoi_MQTT("Serrure/mode_verrou", Verrou_OP, true);
    barre_ferme(0);
    etat_verrou = barre_op;
    Serial.println("Mode verrou: Barré Ouverte");
    finger.ledConfigure(FPMLedControl::Clignotement_Flash, 255, FPMLedColour::Vert);
    finger.ledOn();
    drawOpenLock();
    servo_duty_cycle = (1 * 4095) / 20.0;
    ledcWrite(PWM_servo_CHANNEL, servo_duty_cycle);
  }
}

void barre_ferme(bool activation)
{
  if (activation)
  {
    Envoi_MQTT("Serrure/mode_verrou", Verrou_CL, true);
    barre_ouverte(0);
    etat_verrou = barre_cl;
    Serial.println("Mode verrou: Barré Fermé");
    vider_ecran();
    finger.ledOff();
    servo_duty_cycle = (2 * 4095) / 20.0;
    ledcWrite(PWM_servo_CHANNEL, servo_duty_cycle);
  }
}

void deverrouiallge(bool activation)
{
  barre_ouverte(0);
  barre_ferme(0);
  Serial.println("Verrou mode: déverrouillage");
  finger.ledConfigure(FPMLedControl::Clignotement_Flash, 200, FPMLedColour::Violet, 5);
  finger.ledOff();
  ledcWriteTone(PWM_vibreur_CHANNEL, 0);
  // Tous les cas mènent à verrou_close — switch simplifié
  etat_verrou = verrou_close;
  servo_duty_cycle = (2 * 4095) / 20.0;
  ledcWrite(PWM_servo_CHANNEL, servo_duty_cycle);
}

bool porte_ouverture(void)
{
  switch (etat_verrou)
  {
  case verrou_open:
  {
    // ✅ FIX: Snapshot protégé de count
    portENTER_CRITICAL(&timerMux);
    int local_count = count;
    portEXIT_CRITICAL(&timerMux);

    if (local_count != count_timer_porte_op)
    {
      Serial.println("Ouverture de porte");
      drawOpenLock();
      ledcWriteTone(PWM_vibreur_CHANNEL, PWM_vibreur_FREQ_autorise);
      servo_duty_cycle = (1 * 4095) / 20.0;
      ledcWrite(PWM_servo_CHANNEL, servo_duty_cycle);
      return false;
    }
    else
    {
      Envoi_MQTT("Serrure/mode_verrou", Verrou_CL, true);
      etat_verrou = verrou_close;
      ledcWriteTone(PWM_vibreur_CHANNEL, 0);
      return true;
    }
  }
  break;

  case verrou_close:
  {
    etat_verrou = verrou_open;
    Envoi_MQTT("Serrure/mode_verrou", Verrou_OP, true);

    // ✅ FIX: Snapshot protégé + calcul modulo propre (plus de race condition)
    portENTER_CRITICAL(&timerMux);
    int current_count = count;
    portEXIT_CRITICAL(&timerMux);

    count_timer_porte_op = (current_count + 25) % 50;
    return false;
  }
  break;

  default:
    return true;
    break;
  }
}

uint8_t etat_batterie(uint8_t niveau_batterie_precedent)
{
  uint16_t adc_pin_value = analogRead(adc_pin);
  float voltage = ((float)adc_pin_value / 4095.0f) * 3.3f;
  uint8_t pourcentage = 0;
  if (voltage > 3.0f)
    pourcentage = 100;
  else if (voltage > 2.9f)
    pourcentage = 80;
  else if (voltage > 2.8f)
    pourcentage = 60;
  else if (voltage > 2.7f)
    pourcentage = 50;
  else if (voltage > 2.67f)
    pourcentage = 40;
  else if (voltage > 2.65f)
    pourcentage = 30;
  else if (voltage > 2.6f)
    pourcentage = 20;
  else
    pourcentage = 0;
  if (pourcentage != niveau_batterie_precedent)
  {
    if (voltage > 3.0f)
      return 100;
    else if (voltage > 2.9f)
      return 80;
    else if (voltage > 2.8f)
      return 60;
    else if (voltage > 2.7f)
    {
      Envoi_MQTT("Serrure/log", "Batterie à 50%.", false);
      Envoi_MQTT("Serrure/notification", "{\"title\":\"🔋Batterie🔋\",\"message\":\"Batterie à 50%.\"}", true);
      return 50;
    }
    else if (voltage > 2.67f)
    {
      Envoi_MQTT("Serrure/notification", "{\"title\":\"🔋Batterie🔋\",\"message\":\"Batterie à 40%.\"}", true);
      return 40;
    }
    else if (voltage > 2.65f)
    {
      Envoi_MQTT("Serrure/notification", "{\"title\":\"🔋Batterie🔋\",\"message\":\"Batterie à 30%.\"}", true);
      return 30;
    }
    else if (voltage > 2.6f)
    {
      Envoi_MQTT("Serrure/log", "Batterie critique.", false);
      Envoi_MQTT("Serrure/notification", "{\"title\":\"🔋Batterie critique🔋\",\"message\":\"Veuillez changer la batterie immédiatement.\"}", true);
      return 20;
    }
    else
    {
      Envoi_MQTT("Serrure/log", "Batterie vide.", false);
      Envoi_MQTT("Serrure/notification", "{\"title\":\"🔋Batterie critique🔋\",\"message\":\"Veuillez changer la batterie immédiatement.\"}", true);
      return 0;
    }
  }
  else
  {
    return niveau_batterie_precedent;
  }
}