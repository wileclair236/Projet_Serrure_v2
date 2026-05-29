#include "systeme_fichier.h"
// ── Fonction principale ────────────────────────────────────────────────────
void saveImageToSPIFFS(camera_fb_t* fb) 
{
    if (!fb) {
        Serial.println("Erreur: frame buffer null");
        return;
    }

    // ── 1. Récupérer l'heure courante ──────────────────────────────────────
    struct tm timeinfo;
    char timestamp[20];

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Erreur NTP, utilisation de millis()");
        snprintf(timestamp, sizeof(timestamp), "%lu", millis()); // fallback
    } else {
        // Format: 20250514_143022  (YYYYMMDD_HHMMSS)
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &timeinfo);
    }

    // ── 2. Scanner les fichiers existants ──────────────────────────────────
    int fileCount = 0;
    String oldestName = "";
    unsigned long oldestTime = ULONG_MAX;

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file) {
        String name = String(file.name());
        if (name.startsWith("/img_") && name.endsWith(".jpg")) {
            fileCount++;
            // Extraire YYYYMMDD_HHMMSS pour trouver le plus ancien
            // nom format: /img_20250514_143022.jpg
            String dateStr = name.substring(5, 13) + name.substring(14, 20);
            unsigned long t = strtoul(dateStr.c_str(), nullptr, 10);
            if (t < oldestTime) {
                oldestTime = t;
                oldestName = name;
            }
        }
        file = root.openNextFile();
    }

    // ── 3. Supprimer le plus ancien si on atteint 20 ──────────────────────
    if (fileCount >= 20) {
        SPIFFS.remove(oldestName);
        Serial.printf("Supprimé: %s\n", oldestName.c_str());
    }

    // ── 4. Écrire la nouvelle image ────────────────────────────────────────
    char filename[32];
    snprintf(filename, sizeof(filename), "/img_%s.jpg", timestamp);
    
    File imgFile = SPIFFS.open(filename, FILE_WRITE);

    if (!imgFile) {
        Serial.printf("Erreur: impossible d'ouvrir le fichier en écriture (%s)\n", filename);
        return;
    }

    size_t written = imgFile.write(fb->buf, fb->len);
    imgFile.close();

    if (written == fb->len) {
        Serial.printf("✓ Sauvegardé: %s (%d bytes)\n", filename, written);
    } else {
        Serial.printf("✗ Erreur écriture: %d/%d bytes\n", written, fb->len);
        SPIFFS.remove(filename);
    }
}
bool SystemeFichier::begin()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }
    initialized = true;
    return true;
}

void SystemeFichier::info()
{
    if (!initialized)
    {
        Serial.println("SPIFFS non initialisé");
        return;
    }

    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();

    Serial.println("SPIFFS monté avec succès !");
    Serial.printf("Taille totale: %u bytes\n", totalBytes);
    Serial.printf("Taille utilisée: %u bytes\n", usedBytes);
}
bool SystemeFichier::exists(const char *path)
{
    if (!initialized)
        return false;
    return SPIFFS.exists(path);
}
void SystemeFichier::contenu()
{
    Serial.println("Contenu de SPIFFS :");
    // Ouvrir le répertoire racine
    File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("- échec de l'ouverture du répertoire racine");
        return;
    }
    // Parcourir les fichiers
    File file = root.openNextFile();
    Serial.println("--- Liste des fichiers SPIFFS ---");
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  Taille: ");
            Serial.print(file.size());
            Serial.println(" octets");
        }
        file = root.openNextFile();
    }
    Serial.println("---------------------------------");
}

String SystemeFichier::readFile(const char *path)
{
    if (!initialized)
        return "";

    File file = SPIFFS.open(path);
    if (!file)
    {
        Serial.println("Erreur ouverture fichier");
        return "";
    }

    String content = "";

    while (file.available())
    {
        content += (char)file.read();
    }

    file.close();
    return content;
}

bool SystemeFichier::writeFile(const char *path, const char *message)
{
    if (!initialized)
        return false;

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Erreur ouverture fichier en ecriture");
        return false;
    }

    if (file.print(message))
    {
        Serial.println("Fichier ecrit avec succes");
        file.close();
        return true;
    }
    else
    {
        Serial.println("Erreur ecriture");
        file.close();
        return false;
    }
}
