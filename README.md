# STM32 SX1278 LoRa Module Library
C++ Library for interfacing the SX1278 LoRa Module with a STM32 like the STM32F103C8 using HAL

Based heavily on the Arduino LoRa Library https://github.com/sandeepmistry/arduino-LoRa

**IMPORTANT: The Lib is still incomplete and in testing. Many features are not implemented. Not recommended for use as of right now!**

# USAGE
## Prerequisites
Using the library requires a configured SPI Interface for communication. If you are using the STM CubeIDE, enable the SPI interface by using the .ioc file.

![STM32 CubeIDE SPI settings](/img/CubeIDE_spi.png?raw=true "CubeIDE SPI Interface Configuration")
