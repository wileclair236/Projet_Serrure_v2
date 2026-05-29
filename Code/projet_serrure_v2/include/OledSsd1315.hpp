/**
 * @file OledSsd1315.hpp
 * @brief Fichier d’en-tête principal de la bibliothèque OLED SSD1315
 *
 * Bibliothèque pour piloter des écrans OLED avec contrôleur SSD1315 via I2C.
 *
 * Activation de la bibliothèque :
 *   build_flags = -DOLED_SSD1315_ENABLE=1
 *
 * Si le flag n’est pas défini, la bibliothèque est compilée en mode stub (vide).
 */

#ifndef OLED_SSD1315_HPP
#define OLED_SSD1315_HPP

#include "OledConfig.hpp"
#include "OledTypes.hpp"
#include <cstdint>
#include <cstdarg>
#include <memory>

// Déclarations anticipées spécifiques à la plateforme
#if OLED_USE_ARDUINO
    class TwoWire;
#elif OLED_USE_STM32HAL
    // Inclusion directe du header HAL - le forward declaration entre en conflit avec typedef
    #include "adapters/Stm32HalI2cAdapter.hpp"
#endif

namespace oled {

namespace detail {
    struct OledSsd1315Impl;
}

/**
 * @brief Classe principale pour gérer un OLED SSD1315
 *
 * Regroupe transport, driver et rendu graphique dans une API unique.
 *
 * Exemple d’utilisation (Arduino) :
 * @code
 * #include <Wire.h>
 * #include <oled/OledSsd1315.hpp>
 *
 * OledSsd1315 oled(Wire);
 *
 * void setup() {
 *     Wire.begin();
 *     OledConfig cfg;
 *     cfg.i2cAddr7 = 0x3C;
 *     oled.begin(cfg);
 *     oled.clear();
 *     oled.print("Hello!");
 *     oled.flush();
 * }
 * @endcode
 *
 * Exemple d’utilisation (STM32 HAL) :
 * @code
 * #include <oled/OledSsd1315.hpp>
 *
 * I2C_HandleTypeDef hi2c1;
 * OledSsd1315 oled(&hi2c1);
 *
 * int main() {
 *     HAL_Init();
 *     MX_I2C1_Init();
 *     OledConfig cfg;
 *     cfg.i2cAddr7 = 0x3C;
 *     oled.begin(cfg);
 *     oled.print("Bonjour !");
 *     oled.flush();
 * }
 * @endcode
 */
class OledSsd1315 {
public:
#if OLED_ENABLED
    /**
     * @brief Constructeur par défaut
     */
    OledSsd1315() = default;

    #if OLED_USE_ARDUINO
    /**
     * @brief Constructeur avec Arduino Wire
     * @param wire Référence vers TwoWire (généralement Wire)
     */
    explicit OledSsd1315(TwoWire& wire);
    #endif

    #if OLED_USE_STM32HAL
    /**
     * @brief Constructeur avec I2C STM32 HAL
     * @param hi2c Pointeur vers I2C_HandleTypeDef
     */
    explicit OledSsd1315(I2C_HandleTypeDef* hi2c);
    #endif
#else
    /**
     * @brief Constructeur par défaut (bibliothèque désactivée)
     */
    OledSsd1315() = default;

    /**
     * @brief Constructeur stub (bibliothèque désactivée)
     */
    template<typename T>
    explicit OledSsd1315(T*) {}
    template<typename T>
    explicit OledSsd1315(T&) {}
#endif

    /**
     * @brief Destructeur
     */
    ~OledSsd1315();

    // Interdiction de copie
    OledSsd1315(const OledSsd1315&) = delete;
    OledSsd1315& operator=(const OledSsd1315&) = delete;

    // === Initialisation ===

    /**
     * @brief Initialiser l’écran
     * @param cfg Configuration de l’écran
     * @return OledResult::Ok en cas de succès, OledResult::Disabled si désactivé
     */
    OledResult begin(const OledConfig& cfg);

    /**
     * @brief Vérifier si l’écran est prêt
     */
    bool isReady() const;

    /**
     * @brief Réinitialiser l’état (nécessite un nouvel appel à begin)
     */
    void resetState();

    // === Contrôle de l’écran ===

    /**
     * @brief Allumer / éteindre l’écran
     * @param on true = allumé (0xAF), false = éteint / veille (0xAE)
     */
    OledResult setPower(bool on);

    /**
     * @brief Régler le contraste
     * @param value Niveau de 0 à 255
     */
    OledResult setContrast(uint8_t value);

    /**
     * @brief Inverser les couleurs
     * @param on true = inversion activée
     */
    OledResult invert(bool on);

    // === Buffer ===

    /**
     * @brief Effacer le buffer (tous les pixels éteints)
     */
    void clear();

    /**
     * @brief Remplir le buffer
     * @param color true = tous les pixels allumés
     */
    void fill(bool color);

    /**
     * @brief Envoyer le buffer à l’écran
     */
    OledResult flush();

    // === Primitives graphiques ===

    /**
     * @brief Dessiner un pixel
     */
    void pixel(int x, int y, bool color);

    /**
     * @brief Dessiner une ligne
     */
    void line(int x0, int y0, int x1, int y1, bool color);

    /**
     * @brief Dessiner un rectangle (contour)
     */
    void rect(int x, int y, int w, int h, bool color);

    /**
     * @brief Dessiner un rectangle rempli
     */
    void rectFill(int x, int y, int w, int h, bool color);

    // === Texte ===

    /**
     * @brief Définir la position du curseur
     */
    void setCursor(int x, int y);

    /**
     * @brief Définir la taille du texte (1, 2, 3...)
     */
    void setTextSize(uint8_t scale);

    /**
     * @brief Définir la couleur du texte
     */
    void setTextColor(bool color);

    /**
     * @brief Afficher une chaîne de caractères
     */
    void print(const char* str);

    /**
     * @brief Afficher une chaîne formatée (comme printf)
     * @note Longueur maximale : 128 caractères
     */
    void printf(const char* fmt, ...);

    // === Diagnostic (Phase 1) ===

    /**
     * @brief Obtenir le résultat de la dernière opération
     */
    OledResult getLastResult() const;

    /**
     * @brief Obtenir la description de la dernière erreur
     * @return Chaîne décrivant l’erreur ou nullptr si aucune erreur
     */
    const char* getLastError() const;

    /**
     * @brief Scanner le bus I2C pour trouver l’adresse de l’écran
     * @param startAddr Adresse de début (par défaut 0x3C)
     * @param endAddr Adresse de fin (par défaut 0x3D)
     * @return Adresse trouvée ou 0 si aucune
     */
    uint8_t scanAddress(uint8_t startAddr = 0x3C, uint8_t endAddr = 0x3D);

    // === DMA (Phase 2) ===

#if OLED_USE_STM32HAL
    /**
     * @brief Envoyer le buffer via DMA (non bloquant)
     * @return OledResult::Ok si la transmission démarre
     * @note Nécessite une configuration DMA pour I2C TX
     */
    OledResult flushDMA();

    /**
     * @brief Vérifier si la transmission DMA est terminée
     */
    bool isDMAComplete() const;

    /**
     * @brief Callback de fin de transfert DMA
     *
     * À appeler depuis HAL_I2C_MasterTxCpltCallback :
     * @code
     * void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
     *     display.onDmaComplete();
     * }
     * @endcode
     */
    void onDmaComplete();

    /**
     * @brief Récupération du bus I2C après blocage
     * @param gpioPort Port GPIO (ex: GPIOB)
     * @param sclPin Pin SCL
     * @param sdaPin Pin SDA
     * @return true si récupération réussie
     */
    static bool i2cBusRecovery(void* gpioPort, uint16_t sclPin, uint16_t sdaPin);
#endif

private:
#if OLED_ENABLED
    // pImpl - cache les dépendances plateforme du header public
    detail::OledSsd1315Impl* pImpl_ = nullptr;
#endif
};

} // namespace oled

#endif // OLED_SSD1315_HPP