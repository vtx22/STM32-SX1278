#pragma once

#include "SX1278_reg.hpp"

#include <cstdint>

// Use the following flags for compiling the right library, e.g.: -D STM32F1
#if defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F2)
#include "stm32f2xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#else
#error "Unsupported STM32 microcontroller. Make sure you build with -D STM32F1 for example!"
#endif

#define SET_SX_NSS(value) HAL_GPIO_WritePin(_nss_port, _nss_pin, (value) ? GPIO_PIN_SET : GPIO_PIN_RESET)

class SX1278
{
public:
    SX1278(SPI_HandleTypeDef *hspi);

    void set_reset_pin(GPIO_TypeDef *port, uint16_t pin);
    void set_dio0_pin(GPIO_TypeDef *port, uint16_t pin);
    void set_nss_pin(GPIO_TypeDef *port, uint16_t pin);

    void reset();
    bool init();

    void set_frequency(uint32_t frequency);
private:
    void _write_reg(uint8_t reg, uint8_t value);
    uint8_t _read_reg(uint8_t reg);

    SPI_HandleTypeDef *_hspi;

    GPIO_TypeDef *_rst_port = nullptr;
    GPIO_TypeDef *_dio0_port = nullptr;
    GPIO_TypeDef *_nss_port = nullptr;
    uint16_t _rst_pin, _dio0_pin, _nss_pin;

    uint32_t _frequency = 433e6;
};