# STM32 SX1278 LoRa Module Library
C++ Library for interfacing the SX1278 LoRa Moduke with a STM32 like the STM32F103C8 using HAL

Based heavily on the Arduino LoRa Library https://github.com/sandeepmistry/arduino-LoRa

# USAGE

## Include Library

```
#include "SX1278.hpp"
```
## Initialize SX1278 Object
Initilize with SPI1 and Pin PA4 as Slave Select without Hardware Reset
```
SX1278 sx = SX1278(&hspi1, GPIOA, GPIO_PIN_4);
```
Initilize with SPI1 and Pin PA4 as Slave Select and Pin PB13 as Hardware Reset Pin (connected to RST on the module)
```
SX1278 sx = SX1278(&hspi1, GPIOA, GPIO_PIN_4, GPIOB, GPIO_PIN_13);
```
## Transmit Data
```
char msg[] = "TEST";        //Message to send
sx.beginPacket(0);          //Set SX1278 to Transmit Mode
sx.write((uint8_t*)msg, 4); //Transmit Message (with size = 4 bytes)
sx.endPacket();             //Wait for Transmit to be complete
```
