#ifndef USERS_H
#define USERS_H

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>
#include <fpm.h>

#define MAX_USERS 20

// ================= VARIABLES GLOBALES =================
extern MFRC522 mfrc522; // Déclaration de l'objet MFRC522 pour la lecture RFID
extern bool RFID_irqFlag; // Flag d'interruption

enum TagStatus
{
    NONE,
    WHITELIST,
    BLACKLIST
};

class RFIDTag
{
private:
    uint32_t id_tag;
    uint16_t id_finger;
    String user_name;
    TagStatus status;
    uint8_t num_user;

public:

    RFIDTag();
    RFIDTag(uint32_t uid, uint16_t fid, String name, TagStatus s);
    RFIDTag(uint32_t uid, uint16_t fid, String name, int s);
    uint32_t getID() const;
    uint16_t getIdFinger() const;
    String getUserName() const;
    TagStatus getStatus() const;

    void setID(uint32_t uid);
    void setUserName(String name);
    void setStatus(TagStatus s);

    void printInfo() const;

    // ===== Gestion utilisateurs =====

    static RFIDTag users[MAX_USERS];
    static uint8_t numConfiguredUsers;

    static bool addUser(uint32_t uid, uint16_t fid, String name, TagStatus status);

    static RFIDTag* findUserByUID(uint32_t uid);
    static RFIDTag* findUserByFID(int16_t fid);

    static int getFreeIndex();
    static uint8_t getNumConfiguredUsers();

    static void printAllUsers();
};


// ================= FONCTIONS DE GESTION RFID =================
void IRAM_ATTR RFID_handler(); // Handler d'interruption pour la détection RFID
void setupRFID(int SCK, int MISO, int MOSI, int SS, int IRQ_RFID, int RST); // Initialisation du lecteur RFID
uint32_t Detection_TagsRFID_IRQ(void); // Détection des tags RFID via interruption
String readUserInputWithBackspace(); // Lecture de l'entrée utilisateur avec support de backspace
TagStatus readStatus(); // Lecture du statut de la carte (NONE, WHITELIST, BLACKLIST)


#endif