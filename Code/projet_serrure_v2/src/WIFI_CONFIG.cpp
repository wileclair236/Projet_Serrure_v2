#include "wifi_config.h"

struct tm timeinfo;
WiFiConfig WifiConfig;  // variable globale
String currentSSID;      // Pour readUserInputWithBackspace1

void wifi_setup(const char* ssid_param, const char* password_param, int fuseau_Horaire, const char* hostname_param)
{
    String ssid = ssid_param;
    String password = password_param;
    String hostname = hostname_param;

    // Si aucun SSID ou mot de passe fourni, scan des réseaux disponibles
    if (ssid.length() == 0 || password.length() == 0)
    {
        Serial.println("SSID ou mot de passe WiFi non défini !");
        WiFi.disconnect();
        delay(100);

        Serial.println("Scan des réseaux WiFi...");
        int n = WiFi.scanNetworks();
        if (n == 0)
        {
            Serial.println("Aucun réseau trouvé");
            return;
        }
        Serial.printf("%d réseaux trouvés :\n", n);
        for (int i = 0; i < n; ++i)
        {
            Serial.printf("%d: %s (%d dBm) %s\n",
                          i,
                          WiFi.SSID(i).c_str(),
                          WiFi.RSSI(i),
                          (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Ouvert" : "Protégé");
            delay(10);
        }

        Serial.println("Entrez le numéro du réseau auquel vous souhaitez vous connecter :");
        int networkIndex = -1;
        while (networkIndex == -1)
        {
            if (Serial.available() > 0)
            {
                networkIndex = Serial.parseInt();
                if (networkIndex < 0 || networkIndex >= n)
                {
                    Serial.println("Index invalide. Veuillez entrer un numéro de réseau valide.");
                    networkIndex = -1;
                }
            }
        }
        ssid = WiFi.SSID(networkIndex);
        Serial.println("SSID sélectionné : " + ssid);
        password = readUserInputWithBackspace1(ssid);
    }

    currentSSID = ssid;

    // Connexion WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.setSleep(false);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connexion au WiFi");
    int count_timeout = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        count_timeout++;
        if (count_timeout > 20) // Retry automatique toutes les 10 secondes
        {
            WiFi.begin(ssid.c_str(), password.c_str());
            count_timeout = 0;
            Serial.println("\nNouvelle tentative de connexion WiFi...");
        }
    }
    Serial.println("\nWiFi connecté! IP: " + WiFi.localIP().toString());

    // Hostname mDNS
    if (hostname.length() == 0) hostname = "CAM1";
    if (!MDNS.begin(hostname.c_str()))
    {
        Serial.println("Échec de l'initialisation de mDNS");
    }
    else
    {
        Serial.println("Hostname mDNS initialisé: " + hostname + ".local");
    }

    // Fuseau horaire
    if (fuseau_Horaire == 0xff)
    {
        Serial.println("Fuseau horaire non défini. Entrez votre fuseau horaire (ex: -5 pour EST, 1 pour CET) :");
        while (fuseau_Horaire == 0xff)
        {
            if (Serial.available() > 0)
            {
                
                while (Serial.available() == 0) {}
                int input = Serial.parseInt();
                if (input >= -12 && input <= 14)
                {
                    //fuseau_Horaire = input;
                    ///||||||///
                    fuseau_Horaire = -5; // For testing, set to EST
                    Serial.println("Fuseau horaire défini : " + String(fuseau_Horaire));
                }
                else
                {
                    Serial.println("Fuseau horaire invalide, entrez une valeur entre -12 et +14");
                }
            }
            delay(100);
        }
    }

    // Synchronisation NTP
    configTime(fuseau_Horaire * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Synchronisation de l'heure...");
    while (!getLocalTime(&timeinfo))
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nHeure synchronisée !");
    Serial.println(&timeinfo, "Date et heure actuelles : %A, %B %d %Y %H:%M:%S");

    // Sauvegarde dans la structure globale
    WifiConfig.ssid = ssid;
    WifiConfig.password = password;
    WifiConfig.fuseau_Horaire = fuseau_Horaire;
    WifiConfig.hostname = hostname;
}

// Lecture de l'entrée utilisateur avec gestion du backspace
String readUserInputWithBackspace1(const String& ssid)
{
    Serial.println("Entrez le mot de passe pour le réseau " + ssid + " (appuyez sur Entrée pour valider) :");
    String input = "";
    while (true)
    {
        if (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r') { if (input.length() > 0) break; }
            else if (c == 8 || c == 127) { if (input.length() > 0) { input.remove(input.length() - 1); Serial.print("\b \b"); } }
            else { input += c; Serial.print(c); }
        }
        delay(1);
    }
    Serial.println();
    return input;
}