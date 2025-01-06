#include "SX1278.hpp"

SX1278::SX1278(SPI_HandleTypeDef *hspi) : _hspi(hspi)
{
}

void SX1278::set_reset_pin(GPIO_TypeDef *port, uint16_t pin)
{
    _rst_port = port;
    _rst_pin = pin;
}

void SX1278::set_dio0_pin(GPIO_TypeDef *port, uint16_t pin)
{
    _dio0_port = port;
    _dio0_pin = pin;
}

void SX1278::set_nss_pin(GPIO_TypeDef *port, uint16_t pin)
{
    _nss_port = port;
    _nss_pin = pin;
}

void SX1278::reset()
{
    if (_nss_port == nullptr)
    {
        return;
    }

    HAL_GPIO_WritePin(_rst_port, _rst_pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(_rst_port, _rst_pin, GPIO_PIN_SET);
    HAL_Delay(100);
}

bool SX1278::init()
{
    if (_nss_port == nullptr)
    {
        return false;
    }

    SET_SX_NSS(true);
    reset();

    if (_read_reg(REG_VERSION) != 0x12)
    {
        return false;
    }

    return true;
}

void SX1278::set_frequency(uint32_t frequency)
{
    _frequency = frequency;

    uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

    _write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    _write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
    _write_reg(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

void SX1278::implicit_header_mode()
{
    _implicit_header_mode = 1;

    _write_reg(REG_MODEM_CONFIG_1, _read_reg(REG_MODEM_CONFIG_1) | 0x01);
}

void SX1278::explicit_header_mode()
{
    _implicit_header_mode = 0;
    _write_reg(REG_MODEM_CONFIG_1, _read_reg(REG_MODEM_CONFIG_1) & 0xFE);
}

void SX1278::_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t msg[2] = {reg | 0x80, value};
    SET_SX_NSS(false);
    HAL_SPI_Transmit(_hspi, msg, 2, SX1278_SPI_MAX_DELAY);
    SET_SX_NSS(true);
}

uint8_t SX1278::_read_reg(uint8_t reg)
{
    uint8_t msg = reg & 0x7F;
    uint8_t value = 0;
    SET_SX_NSS(false);
    HAL_SPI_Transmit(_hspi, &msg, 1, SX1278_SPI_MAX_DELAY);
    HAL_SPI_Receive(_hspi, &value, 1, SX1278_SPI_MAX_DELAY);
    SET_SX_NSS(true);
    return value;
}
