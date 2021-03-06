# STM32 SX1278 LoRa Module Library
C++ Library for interfacing the SX1278 LoRa Module with a STM32 like the STM32F103C8 using HAL

Based heavily on the Arduino LoRa Library https://github.com/sandeepmistry/arduino-LoRa

**IMPORTANT: The Lib is still incomplete and in testing. Many features are not implemented. Not recommended for use as of right now!**

# USAGE
## Prerequisites
Using the library requires a configured SPI Interface for communication. If you are using the STM CubeIDE enable the SPI interface by using the .ioc file.

![Alt text](/img/CubeIDE_spi.png?raw=true "CubeIDE SPI Interface Configuration")

## Include Library

```C++
#include "SX1278.hpp"
```
## Initialize SX1278 Object
Initialize with SPI1 and Pin PA4 as Slave Select without Hardware Reset (connect RST to the STM RST Pin)
```C++
SX1278 sx = SX1278(&hspi1, GPIOA, GPIO_PIN_4);
```
Initialize with SPI1 and Pin PA4 as Slave Select and Pin PB13 as Hardware Reset Pin (connected to RST on the module)
```C++
SX1278 sx = SX1278(&hspi1, GPIOA, GPIO_PIN_4, GPIOB, GPIO_PIN_13);
```
## Transmit Data
```C++
char msg[] = "TEST";        // Message to send
sx.beginPacket(0);          // Set SX1278 to Transmit Mode
sx.write(msg, 4);           // Transmit Message (with size = 4 bytes)
sx.endPacket(false);        // Wait for Transmit to be complete, with async = false --> endPacket blocks until transmit is complete
```
The following write functions are usable:
```C++
   size_t write(const uint8_t *buffer, size_t size);
   size_t write(const char *buffer, size_t size);
   size_t write(std::vector<uint8_t> data);
   size_t write(std::vector<char> data);
```
