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
}

bool SX1278::init()
{
    if (_nss_port == nullptr)
    {
        return false;
    }

    SET_SX_NNS(true);
    reset();
}

void SX1278::_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t msg[2] = {reg | 0x80, value};
    SET_SX_NNS(false);
    HAL_SPI_Transmit(_hspi, msg, 2, 1);
    SET_SX_NNS(true);
}