#include "carte_sd.h"

bool CarteSD::begin(int clk, int cmd, int d0)
{
    SD_MMC.setPins(clk, cmd, d0);

    while (!SD_MMC.begin("/sdcard", true))
    {
        Serial.println("Erreur montage SD_MMC...");
        delay(1000);
    }

    Serial.println("SD_MMC monté avec succès");
    initialized = true;

    return true;
}

void CarteSD::info()
{
    if (!initialized)
    {
        Serial.println("SD non initialisée");
        return;
    }

    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("Aucune carte SD detectee");
        return;
    }

    Serial.println("Carte SD detectee !");
    Serial.printf("Taille: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
}

bool CarteSD::createDir(const char *path)
{
    if (!initialized) return false;

    if (SD_MMC.exists(path))
        return true;

    if (SD_MMC.mkdir(path))
    {
        Serial.println("Dossier cree");
        return true;
    }
    else
    {
        Serial.println("Erreur creation dossier");
        return false;
    }
}

bool CarteSD::exists(const char *path)
{
    if (!initialized) return false;
    return SD_MMC.exists(path);
}

String CarteSD::readFile(const char *path)
{
    if (!initialized) return "";

    File file = SD_MMC.open(path);
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

bool CarteSD::writeFile(const char *path, const char *message)
{
    if (!initialized) return false;

    File file = SD_MMC.open(path, FILE_WRITE);
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