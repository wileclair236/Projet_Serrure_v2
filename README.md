# Serrure Intelligente Connectée

## Description

Ce projet consiste en la conception et la réalisation d'une serrure intelligente connectée permettant le contrôle d'accès d'une porte à l'aide de plusieurs méthodes d'authentification. La serrure peut être commandée localement ou à distance via Home Assistant.

Le système est composé de plusieurs circuits imprimés (PCB) assurant la gestion des accès, les communications sans fil, l'acquisition d'images, le stockage des événements et le contrôle du mécanisme de verrouillage.

## Fonctionnalités

* Déverrouillage par carte RFID
* Déverrouillage par empreinte digitale
* Déverrouillage à distance via Home Assistant
* Déverrouillage mécanique par clé
* Notifications en temps réel
* Capture d'images lors des événements d'accès
* Journalisation des accès
* Gestion de plusieurs utilisateurs
* Contrôle d'un servomoteur pour le verrouillage et le déverrouillage
* Communication sans fil via Wi-Fi et MQTT

## Architecture du système

### PCB principal

Le PCB principal assure le contrôle complet de la serrure :

* Gestion des communications avec Home Assistant
* Gestion des utilisateurs autorisés
* Traitement des demandes d'accès
* Contrôle du servomoteur
* Acquisition et stockage des données
* Gestion des capteurs et périphériques

### PCB de la clé mécanique

Le PCB secondaire agit comme un interrupteur relié au PCB principal. Lorsqu'une clé est insérée puis tournée, il transmet un signal d'ouverture au contrôleur principal afin de permettre l'accès.

## Matériel utilisé

* ESP32-S3
* Lecteur RFID MFRC522
* Capteur d'empreintes digitales R503
* Caméra OV2640
* Écran OLED
* Servomoteur
* Carte microSD
* Capteur de proximité Vcnl36825t
* Batterie AA

## Logiciels utilisés

* PlatformIO
* Home Assistant
* MQTT
* Git
* KiCad

## Communication MQTT

Le système utilise MQTT pour transmettre les informations entre la serrure et Home Assistant.

## Sécurité

* Gestion des utilisateurs autorisés
* Journalisation des accès
* Authentification par plusieurs méthodes
* Surveillance des événements en temps réel
* Notifications automatiques vers Home Assistant

## Résultats

La serrure permet un contrôle d'accès sécurisé et connecté tout en conservant une solution de secours mécanique grâce à la clé physique. L'intégration avec Home Assistant offre une supervision en temps réel ainsi qu'une gestion centralisée des accès.

## Auteur

Wilkens Pinthière
Projet de conception d'une serrure intelligente connectée.
