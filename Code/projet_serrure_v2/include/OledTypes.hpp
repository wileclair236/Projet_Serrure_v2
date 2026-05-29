/**
 * @file OledTypes.hpp
 * @brief Общие типы и перечисления для OLED SSD1315
 * 
 * Библиотека OLED SSD1315 для PlatformIO
 * Флаг включения: OLED_SSD1315_ENABLE=1
 */

#ifndef OLED_TYPES_HPP
#define OLED_TYPES_HPP

#include "OledConfig.hpp"
#include <cstdint>

namespace oled {

/**
 * @brief Результаты операций OLED
 */
enum class OledResult {
    Ok,             // Операция успешна
    Disabled,       // Библиотека отключена (флаг не задан)
    I2cError,       // Ошибка I2C
    NotInitialized, // Дисплей не инициализирован
    InvalidArg,     // Неверный аргумент
    Unsupported,    // Операция не поддерживается
    Busy,           // Устройство занято (DMA в процессе)
    Timeout         // Таймаут операции
};

/**
 * @brief Режим питания дисплея
 * 
 * InternalChargePump - используется встроенная "помпа" (charge pump)
 * ExternalVcc - внешнее питание VCC
 */
enum class VccMode {
    InternalChargePump, // Внутренний charge pump (0x8D, 0x14)
    ExternalVcc         // Внешнее VCC (0x8D, 0x10)
};

/**
 * @brief Тип callback для управления GPIO reset
 * @param high true = установить HIGH, false = установить LOW
 * 
 * Пример для Arduino:
 * @code
 * void myResetCallback(bool high) {
 *     digitalWrite(RST_PIN, high ? HIGH : LOW);
 * }
 * @endcode
 * 
 * Пример для STM32 HAL:
 * @code
 * void myResetCallback(bool high) {
 *     HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
 * }
 * @endcode
 */
using ResetGpioCallback = void (*)(bool high);

/**
 * @brief Конфигурация OLED дисплея
 */
struct OledConfig {
    uint8_t  i2cAddr7 = 0x3C;      // 7-битный адрес (0x3C или 0x3D)
    uint16_t width    = 128;       // Ширина в пикселях
    uint16_t height   = 64;        // Высота в пикселях (64 или 32)
    uint32_t i2cFreq  = 400000;    // Частота I2C (опционально)
    VccMode  vccMode  = VccMode::InternalChargePump;
    bool     flip180  = false;     // Поворот на 180 градусов
    
    /**
     * @brief Callback для аппаратного reset (platform-agnostic)
     * 
     * Если nullptr — используется только задержка для стабилизации.
     * Если задан — выполняется последовательность: HIGH → LOW → HIGH.
     */
    ResetGpioCallback resetCallback = nullptr;
};

} // namespace oled

#endif // OLED_TYPES_HPP
