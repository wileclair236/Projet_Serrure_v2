#include "Users.h"
MFRC522 mfrc522;
// Définition des variables globales
bool RFID_irqFlag = false; // Flag to indicate an RFID event occurred

RFIDTag RFIDTag::users[MAX_USERS];
uint8_t RFIDTag::numConfiguredUsers = 0;

RFIDTag::RFIDTag()
{
  id_tag = 0;
  id_finger = 0xFFFF;
  user_name = "";
  status = NONE;
}

RFIDTag::RFIDTag(uint32_t uid, uint16_t fid, String name, TagStatus s)
{
  id_tag = uid;
  id_finger = fid;
  user_name = name;
  status = s;
  numConfiguredUsers++;
}
RFIDTag::RFIDTag(uint32_t uid, uint16_t fid, String name, int s)
{
  id_tag = uid;
  id_finger = fid;
  user_name = name;
  status = (TagStatus)s;
  numConfiguredUsers++;
}

uint32_t RFIDTag::getID() const
{
  return id_tag;
}

uint16_t RFIDTag::getIdFinger() const
{
  return id_finger;
}

String RFIDTag::getUserName() const
{
  return user_name;
}

TagStatus RFIDTag::getStatus() const
{
  return status;
}

void RFIDTag::setID(uint32_t uid)
{
  id_tag = uid;
}

void RFIDTag::setUserName(String name)
{
  user_name = name;
}

void RFIDTag::setStatus(TagStatus s)
{
  status = s;
}

void RFIDTag::printInfo() const
{
  Serial.println("\n\r");
  Serial.println("========================");
  Serial.println("||= USER INFORMATION =||");
  Serial.println("========================");
  Serial.print("UID: ");
  Serial.println(id_tag, HEX);

  Serial.print("User: ");
  Serial.println(user_name);

  Serial.print("Finger ID: ");
  Serial.println(id_finger);

  Serial.print("Status: ");

  switch (status)
  {
  case NONE:
    Serial.println("NONE");
    break;

  case WHITELIST:
    Serial.println("WHITELIST");
    break;

  case BLACKLIST:
    Serial.println("BLACKLIST");
    break;
  }
  Serial.println("====================");
  Serial.println("====================");
  Serial.println("\n\r");
}

int RFIDTag::getFreeIndex()
{
  for (int i = 0; i < MAX_USERS; i++)
  {
    if (users[i].getID() == 0)
    {
      return i;
    }
  }

  return -1;
}

uint8_t RFIDTag::getNumConfiguredUsers()
{
  return numConfiguredUsers;
}

bool RFIDTag::addUser(uint32_t uid, uint16_t fid, String name, TagStatus status)
{
  int index = getFreeIndex();

  if (index == -1)
  {
    Serial.println("❌ Tableau utilisateurs plein");
    return false;
  }
  numConfiguredUsers++;
  users[index] = RFIDTag(uid, fid, name, status);

  Serial.println("✅ Utilisateur ajouté");

  return true;
}

RFIDTag *RFIDTag::findUserByUID(uint32_t uid)
{
  for (int i = 0; i < MAX_USERS; i++)
  {
    if (users[i].getID() == uid)
    {
      return &users[i];
    }
  }

  return nullptr;
}

RFIDTag *RFIDTag::findUserByFID(int16_t fid)
{
  for (int i = 0; i < MAX_USERS; i++)
  {
    if (users[i].getIdFinger() == fid)
    {
      return &users[i];
    }
  }

  return nullptr;
}

void RFIDTag::printAllUsers()
{
  Serial.println("===== UTILISATEURS =====");

  for (int i = 0; i < MAX_USERS; i++)
  {
    if (users[i].getID() != 0)
    {
      users[i].printInfo();
    }
  }

  Serial.println("========================");
}

void IRAM_ATTR RFID_handler()
{
  RFID_irqFlag = true; // Set the flag to indicate an RFID event occurred
}

void setupRFID(int SCK, int MISO, int MOSI, int SS_rfid, int IRQ_RFID, int RST)
{
  SPI.begin(SCK, MISO, MOSI); // Initialisation du bus SPI
  SPI.setFrequency(400000);   // Fréquence à 400 kHz pour le RFID

  mfrc522.PCD_Init(SS_rfid, RST); // Initialisation du lecteur RFID avec les pins SS et RST
  mfrc522.PCD_AntennaOn();        // Allumer l'antenne

  pinMode(IRQ_RFID, INPUT_PULLUP);                                         // Mode d'entrée pour l'IRQ
  attachInterrupt(digitalPinToInterrupt(IRQ_RFID), RFID_handler, FALLING); // Attacher l'interruption

  // Activer les interruptions et les flags nécessaires
  mfrc522.PCD_WriteRegister(MFRC522::ComIEnReg, 0xA0);               // Active RxIRq et IRQ inversé (LOW actif)
  mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F);               // Clear flags
  mfrc522.PCD_WriteRegister(MFRC522::CommandReg, MFRC522::PCD_Idle); // Mettre en mode Idle

  Serial.println("RFID Reader Initialized");
}

// Fonction de détection des tags RFID via interruption
uint32_t Detection_TagsRFID_IRQ(void) // Renommé pour corriger la faute
{
  byte atqa[2];
  byte size = sizeof(atqa);
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
  MFRC522::StatusCode status = mfrc522.PICC_RequestA(atqa, &size);
  if (RFID_irqFlag) // Si une interruption a été déclenchée
  {
    Serial.println("Interruption RFID détectée");
    uint32_t RFID_Tag = 0;
    RFID_irqFlag = false;                                // Réinitialiser le flag d'interruption
    mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F); // Clear flags après traitemen

    if (status == MFRC522::STATUS_OK)
    {
      Serial.printf("=== CARD DETECTED ===\n");

      MFRC522::Uid uid;
      status = mfrc522.PICC_Select(&uid); // Sélectionner la carte RFID et lire l'UID
      Serial.print("UID détecté: ");

      for (byte i = 0; i < uid.size; i++)
      {
        Serial.printf("%02X", uid.uidByte[i]);
        RFID_Tag = (RFID_Tag << 8) | uid.uidByte[i]; // Convertir l'UID en uint32_t
      }
      Serial.println("\n-------------------");
      mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F);
      mfrc522.PCD_WriteRegister(MFRC522::CommandReg, MFRC522::PCD_Idle); // Mettre en mode Idle
      mfrc522.PICC_HaltA();                                              // Mettre la carte en veille
      return RFID_Tag;
    }
    else
    {
      Serial.println("Erreur de lecture RFID");
      return 0x0; // Aucune carte détectée
    }
  }
  else
  {
    // Serial.println("Aucune interruption RFID détectée");
    return 0x0; // Pas d'interruption
  }
}

// Lecture de l'entrée utilisateur avec gestion du backspace
String readUserInputWithBackspace()
{
  Serial.println("Entrez le nom de l'utilisateur (appuyez sur Entrée pour valider) :");
  String input = "";

  while (true)
  {
    if (Serial.available() > 0)
    {
      char c = Serial.read();

      if (c == '\n' || c == '\r')
      { // fin de saisie
        if (input.length() > 0)
          break;
      }
      else if (c == 8 || c == 127)
      { // Backspace (8 = BS, 127 = DEL)
        if (input.length() > 0)
        {
          input.remove(input.length() - 1); // supprime le dernier caractère
          Serial.print("\b \b");            // efface aussi sur le moniteur série
        }
      }
      else
      {
        input += c;      // ajoute le caractère
        Serial.print(c); // affiche le caractère sur le moniteur
      }
    }
    delay(1);
  }

  Serial.println(); // nouvelle ligne après fin de saisie
  return input;
}

TagStatus readStatus()
{
  Serial.println("Entrez le statut du tag (0 = NONE, 1 = WHITELIST, 2 = BLACKLIST) :");
  while (true)
  {
    if (Serial.available() > 0)
    {
      char c = Serial.read();
      if (c == '0')
        return NONE;
      else if (c == '1')
        return WHITELIST;
      else if (c == '2')
        return BLACKLIST;
      else
        Serial.println("Entrée invalide. Veuillez entrer 0, 1 ou 2.");
    }
    delay(1);
  }
}
