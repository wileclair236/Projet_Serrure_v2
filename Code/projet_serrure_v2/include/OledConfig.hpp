/**
 * @file OledConfig.hpp
 * @brief Единая конфигурация библиотеки OLED SSD1315
 *
 * Все файлы библиотеки должны включать этот заголовок для получения
 * флага OLED_ENABLED, определения платформы и базовых настроек.
 *
 * Поддерживаемые платформы:
 * - Arduino (Wire) - автоопределение или -DOLED_PLATFORM_ARDUINO=1
 * - STM32 HAL      - -DOLED_PLATFORM_STM32HAL=1
 * - ESP-IDF        - -DOLED_PLATFORM_ESPIDF=1 (будущее)
 */

#ifndef OLED_CONFIG_HPP
#define OLED_CONFIG_HPP

// === Флаг включения библиотеки ===
// Определяется в build_flags: -DOLED_SSD1315_ENABLE=1
#if defined(OLED_SSD1315_ENABLE) && OLED_SSD1315_ENABLE
    #define OLED_ENABLED 1
#else
    #define OLED_ENABLED 0
#endif

// === Автоопределение платформы ===
// Приоритет: явный флаг > автодетекция

// Сбрасываем все платформы
#ifndef OLED_USE_ARDUINO
    #define OLED_USE_ARDUINO 0
#endif
#ifndef OLED_USE_STM32HAL
    #define OLED_USE_STM32HAL 0
#endif
// ESP-IDF поддержка (PLANNED - не реализовано)
#ifndef OLED_USE_ESPIDF
    #define OLED_USE_ESPIDF 0
#endif

// Явное указание платформы через build_flags
#if defined(OLED_PLATFORM_STM32HAL) && OLED_PLATFORM_STM32HAL
    #undef OLED_USE_STM32HAL
    #define OLED_USE_STM32HAL 1
#elif defined(OLED_PLATFORM_ESPIDF) && OLED_PLATFORM_ESPIDF
    #undef OLED_USE_ESPIDF
    #define OLED_USE_ESPIDF 1
#elif defined(OLED_PLATFORM_ARDUINO) && OLED_PLATFORM_ARDUINO
    #undef OLED_USE_ARDUINO
    #define OLED_USE_ARDUINO 1
#elif defined(ARDUINO)
    // Автодетекция Arduino
    #undef OLED_USE_ARDUINO
    #define OLED_USE_ARDUINO 1
#endif

// Проверка: хотя бы одна платформа должна быть определена
#if OLED_ENABLED && !OLED_USE_ARDUINO && !OLED_USE_STM32HAL && !OLED_USE_ESPIDF
    #error "OLED: No platform defined. Use -DOLED_PLATFORM_ARDUINO=1, -DOLED_PLATFORM_STM32HAL=1 or -DOLED_PLATFORM_ESPIDF=1"
#endif

// === Размеры буферов ===
#ifndef OLED_MAX_WIDTH
    #define OLED_MAX_WIDTH 128
#endif

#ifndef OLED_MAX_HEIGHT
    #define OLED_MAX_HEIGHT 64
#endif

// Максимальный размер framebuffer (128x64 / 8 = 1024 байт)
#define OLED_MAX_BUFFER_SIZE ((OLED_MAX_WIDTH * OLED_MAX_HEIGHT) / 8)

// Размер буфера для printf
#define OLED_PRINTF_BUFFER_SIZE 128

// === Wire buffer size ===
#ifndef OLED_WIRE_BUFFER_SIZE
    #define OLED_WIRE_BUFFER_SIZE 32
#endif

// === I2C Chunk Size ===
// STM32 HAL может передавать большие блоки, Arduino Wire ограничен 32 байтами
#if OLED_USE_STM32HAL
    #ifndef OLED_I2C_CHUNK_SIZE
        #define OLED_I2C_CHUNK_SIZE 128
    #endif
#else
    #ifndef OLED_I2C_CHUNK_SIZE
        #define OLED_I2C_CHUNK_SIZE 16
    #endif
#endif

#endif // OLED_CONFIG_HPP
