/*
 * @file OledSsd1315.cpp
 * @brief Реализация главного класса OledSsd1315
 */

#include "oled/OledSsd1315.hpp"
#include "oled/OledSsd1315Impl.hpp"

#if OLED_ENABLED

#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace oled {

namespace {
    // I2C bus recovery constants
    constexpr int kI2cRecoveryClockPulses = 9;      // Макс. кол-во clock pulses для восстановления
    constexpr int kI2cBitBangDelayLoops = 100;      // Циклы задержки для bit-banging (~несколько мкс)
    constexpr uint8_t kI2cDataCommandPrefix = 0x40; // Префикс команды данных для GDDRAM
} // anonymous namespace

#if OLED_USE_ARDUINO
OledSsd1315::OledSsd1315(TwoWire& wire) : pImpl_(new detail::OledSsd1315Impl()) {
    pImpl_->wire = &wire;
}
#endif

#if OLED_USE_STM32HAL
OledSsd1315::OledSsd1315(I2C_HandleTypeDef* hi2c) : pImpl_(new detail::OledSsd1315Impl()) {
    pImpl_->hi2c = hi2c;
}
#endif

OledSsd1315::~OledSsd1315() {
    delete pImpl_;
}

OledResult OledSsd1315::begin(const OledConfig& cfg) {
    if (!pImpl_) {
        pImpl_ = new detail::OledSsd1315Impl();
    }

    // Сброс состояния
    resetState();

    // Проверка размера буфера
    size_t bufSize = static_cast<size_t>(cfg.width) * cfg.height / 8;
    if (bufSize > OLED_MAX_BUFFER_SIZE) {
        return OledResult::InvalidArg;
    }

    // Инициализируем адаптер I2C (platform-specific)
    #if OLED_USE_ARDUINO
    if (!pImpl_->wire) {
        return OledResult::InvalidArg;
    }
    pImpl_->adapter.init(*pImpl_->wire);
    #elif OLED_USE_STM32HAL
    if (!pImpl_->hi2c) {
        return OledResult::InvalidArg;
    }
    pImpl_->adapter.init(pImpl_->hi2c);
    #endif

    // Инициализируем драйвер
    OledResult res = pImpl_->driver.init(pImpl_->adapter, cfg);
    if (res != OledResult::Ok) {
        pImpl_->lastResult = res;
        pImpl_->lastErrorMsg = "Driver init failed";
        return res;
    }

    // Инициализируем графический контекст
    pImpl_->gfx.init(pImpl_->buffer, cfg.width, cfg.height);

    // Очищаем буфер
    pImpl_->gfx.clear();

    pImpl_->initialized = true;
    pImpl_->lastResult = OledResult::Ok;
    pImpl_->lastErrorMsg = nullptr;
    return OledResult::Ok;
}

bool OledSsd1315::isReady() const {
    return pImpl_ && pImpl_->initialized && pImpl_->driver.isReady();
}

void OledSsd1315::resetState() {
    if (pImpl_) {
        pImpl_->initialized = false;
    }
}

OledResult OledSsd1315::setPower(bool on) {
    if (!isReady()) {
        if (pImpl_) {
            pImpl_->lastResult = OledResult::NotInitialized;
            pImpl_->lastErrorMsg = "Display not initialized";
        }
        return OledResult::NotInitialized;
    }
    pImpl_->lastResult = pImpl_->driver.setPower(on);
    pImpl_->lastErrorMsg = (pImpl_->lastResult != OledResult::Ok) ? "setPower failed" : nullptr;
    return pImpl_->lastResult;
}

OledResult OledSsd1315::setContrast(uint8_t value) {
    if (!isReady()) {
        if (pImpl_) {
            pImpl_->lastResult = OledResult::NotInitialized;
            pImpl_->lastErrorMsg = "Display not initialized";
        }
        return OledResult::NotInitialized;
    }
    pImpl_->lastResult = pImpl_->driver.setContrast(value);
    pImpl_->lastErrorMsg = (pImpl_->lastResult != OledResult::Ok) ? "setContrast failed" : nullptr;
    return pImpl_->lastResult;
}

OledResult OledSsd1315::invert(bool on) {
    if (!isReady()) {
        if (pImpl_) {
            pImpl_->lastResult = OledResult::NotInitialized;
            pImpl_->lastErrorMsg = "Display not initialized";
        }
        return OledResult::NotInitialized;
    }
    pImpl_->lastResult = pImpl_->driver.setInvert(on);
    pImpl_->lastErrorMsg = (pImpl_->lastResult != OledResult::Ok) ? "invert failed" : nullptr;
    return pImpl_->lastResult;
}

void OledSsd1315::clear() {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.clear();
    }
}

void OledSsd1315::fill(bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.fill(color);
    }
}

OledResult OledSsd1315::flush() {
    if (!isReady()) {
        if (pImpl_) {
            pImpl_->lastResult = OledResult::NotInitialized;
            pImpl_->lastErrorMsg = "Display not initialized";
        }
        return OledResult::NotInitialized;
    }
    pImpl_->lastResult = pImpl_->driver.writeBuffer(pImpl_->gfx.buffer(), pImpl_->gfx.bufferSize());
    pImpl_->lastErrorMsg = (pImpl_->lastResult != OledResult::Ok) ? "flush failed" : nullptr;
    return pImpl_->lastResult;
}

void OledSsd1315::pixel(int x, int y, bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.pixel(x, y, color);
    }
}

void OledSsd1315::line(int x0, int y0, int x1, int y1, bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.line(x0, y0, x1, y1, color);
    }
}

void OledSsd1315::rect(int x, int y, int w, int h, bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.rect(x, y, w, h, color);
    }
}

void OledSsd1315::rectFill(int x, int y, int w, int h, bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.rectFill(x, y, w, h, color);
    }
}

void OledSsd1315::setCursor(int x, int y) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.setCursor(x, y);
    }
}

void OledSsd1315::setTextSize(uint8_t scale) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.setTextSize(scale);
    }
}

void OledSsd1315::setTextColor(bool color) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.setTextColor(color);
    }
}

void OledSsd1315::print(const char* str) {
    if (pImpl_ && pImpl_->gfx.isInitialized()) {
        pImpl_->gfx.print(str);
    }
}

void OledSsd1315::printf(const char* fmt, ...) {
    if (!pImpl_ || !pImpl_->gfx.isInitialized()) return;

    char buf[OLED_PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    pImpl_->gfx.print(buf);
}

// === Диагностика (Фаза 1) ===

OledResult OledSsd1315::getLastResult() const {
    return pImpl_ ? pImpl_->lastResult : OledResult::NotInitialized;
}

const char* OledSsd1315::getLastError() const {
    return pImpl_ ? pImpl_->lastErrorMsg : nullptr;
}

uint8_t OledSsd1315::scanAddress(uint8_t startAddr, uint8_t endAddr) {
    if (!pImpl_ || startAddr > endAddr) {
        return 0;
    }

    for (uint8_t addr = startAddr; addr <= endAddr; addr++) {
        if (pImpl_->adapter.probe(addr)) {
            return addr;
        }
    }

    return 0;  // Не найден
}

// === STM32 HAL специфичные методы (Фаза 2) ===

#if OLED_USE_STM32HAL

OledResult OledSsd1315::flushDMA() {
    if (!isReady()) {
        if (pImpl_) {
            pImpl_->lastResult = OledResult::NotInitialized;
            pImpl_->lastErrorMsg = "Display not initialized";
        }
        return OledResult::NotInitialized;
    }

    if (pImpl_->dmaInProgress) {
        pImpl_->lastResult = OledResult::Busy;
        pImpl_->lastErrorMsg = "DMA transfer in progress";
        return pImpl_->lastResult;
    }

    // Получаем конфигурацию для адреса
    const OledConfig& cfg = pImpl_->driver.config();
    uint16_t addr8 = static_cast<uint16_t>(cfg.i2cAddr7) << 1;

    // Подготовка буфера с командой данных
    // Для DMA нужен отдельный буфер с префиксом 0x40
    static uint8_t dmaBuffer[OLED_MAX_BUFFER_SIZE + 1];
    dmaBuffer[0] = kI2cDataCommandPrefix;
    memcpy(dmaBuffer + 1, pImpl_->gfx.buffer(), pImpl_->gfx.bufferSize());

    pImpl_->dmaInProgress = true;

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(
        pImpl_->hi2c,
        addr8,
        dmaBuffer,
        static_cast<uint16_t>(pImpl_->gfx.bufferSize() + 1)
    );

    if (status != HAL_OK) {
        pImpl_->dmaInProgress = false;
        pImpl_->lastResult = OledResult::I2cError;
        pImpl_->lastErrorMsg = "DMA transfer start failed";
        return pImpl_->lastResult;
    }

    pImpl_->lastResult = OledResult::Ok;
    pImpl_->lastErrorMsg = nullptr;
    return pImpl_->lastResult;
}

bool OledSsd1315::isDMAComplete() const {
    return !pImpl_ || !pImpl_->dmaInProgress;
}

void OledSsd1315::onDmaComplete() {
    if (pImpl_) {
        pImpl_->dmaInProgress = false;
    }
}

bool OledSsd1315::i2cBusRecovery(void* gpioPort, uint16_t sclPin, uint16_t sdaPin) {
    if (!gpioPort) {
        return false;
    }

    GPIO_TypeDef* port = static_cast<GPIO_TypeDef*>(gpioPort);

    // Сохраняем текущий режим пинов
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    // Настраиваем SCL как выход
    gpio.Pin = sclPin;
    HAL_GPIO_Init(port, &gpio);

    // Проверяем SDA - если LOW, генерируем clock pulses
    bool recovered = false;

    for (int i = 0; i < kI2cRecoveryClockPulses; i++) {
        // Генерируем clock pulse
        HAL_GPIO_WritePin(port, sclPin, GPIO_PIN_RESET);
        for (volatile int d = 0; d < kI2cBitBangDelayLoops; d++) {}

        HAL_GPIO_WritePin(port, sclPin, GPIO_PIN_SET);
        for (volatile int d = 0; d < kI2cBitBangDelayLoops; d++) {}

        // Проверяем SDA
        if (HAL_GPIO_ReadPin(port, sdaPin) == GPIO_PIN_SET) {
            recovered = true;
            break;
        }
    }

    // Генерируем STOP condition
    gpio.Pin = sdaPin;
    HAL_GPIO_Init(port, &gpio);

    HAL_GPIO_WritePin(port, sdaPin, GPIO_PIN_RESET);
    for (volatile int d = 0; d < kI2cBitBangDelayLoops; d++) {}
    HAL_GPIO_WritePin(port, sclPin, GPIO_PIN_SET);
    for (volatile int d = 0; d < kI2cBitBangDelayLoops; d++) {}
    HAL_GPIO_WritePin(port, sdaPin, GPIO_PIN_SET);

    return recovered;
}

#endif // OLED_USE_STM32HAL

} // namespace oled

#else // OLED_ENABLED == 0

// === Заглушки когда библиотека отключена ===

namespace oled {

OledSsd1315::~OledSsd1315() {}

OledResult OledSsd1315::begin(const OledConfig&) {
    return OledResult::Disabled;
}

bool OledSsd1315::isReady() const {
    return false;
}

void OledSsd1315::resetState() {}

OledResult OledSsd1315::setPower(bool) {
    return OledResult::Disabled;
}

OledResult OledSsd1315::setContrast(uint8_t) {
    return OledResult::Disabled;
}

OledResult OledSsd1315::invert(bool) {
    return OledResult::Disabled;
}

void OledSsd1315::clear() {}

void OledSsd1315::fill(bool) {}

OledResult OledSsd1315::flush() {
    return OledResult::Disabled;
}

void OledSsd1315::pixel(int, int, bool) {}

void OledSsd1315::line(int, int, int, int, bool) {}

void OledSsd1315::rect(int, int, int, int, bool) {}

void OledSsd1315::rectFill(int, int, int, int, bool) {}

void OledSsd1315::setCursor(int, int) {}

void OledSsd1315::setTextSize(uint8_t) {}

void OledSsd1315::setTextColor(bool) {}

void OledSsd1315::print(const char*) {}

void OledSsd1315::printf(const char*, ...) {}

OledResult OledSsd1315::getLastResult() const {
    return OledResult::Disabled;
}

const char* OledSsd1315::getLastError() const {
    return "Library disabled";
}

uint8_t OledSsd1315::scanAddress(uint8_t, uint8_t) {
    return 0;
}

} // namespace oled


#endif // OLED_ENABLED
