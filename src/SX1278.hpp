#pragma once
#include "stm32f1xx_hal.h"

#define REG_FIFO 0x00
#define REG_OP_MODE 0x01
#define REG_FRF_MSB 0x06
#define REG_FRF_MID 0x07
#define REG_FRF_LSB 0x08
#define REG_PA_CONFIG 0x09
#define REG_OCP 0x0b
#define REG_LNA 0x0c
#define REG_FIFO_ADDR_PTR 0x0d
#define REG_FIFO_TX_BASE_ADDR 0x0e
#define REG_FIFO_RX_BASE_ADDR 0x0f
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS 0x12
#define REG_RX_NB_BYTES 0x13
#define REG_PKT_SNR_VALUE 0x19
#define REG_PKT_RSSI_VALUE 0x1a
#define REG_RSSI_VALUE 0x1b
#define REG_MODEM_CONFIG_1 0x1d
#define REG_MODEM_CONFIG_2 0x1e
#define REG_PREAMBLE_MSB 0x20
#define REG_PREAMBLE_LSB 0x21
#define REG_PAYLOAD_LENGTH 0x22
#define REG_MODEM_CONFIG_3 0x26
#define REG_FREQ_ERROR_MSB 0x28
#define REG_FREQ_ERROR_MID 0x29
#define REG_FREQ_ERROR_LSB 0x2a
#define REG_RSSI_WIDEBAND 0x2c
#define REG_DETECTION_OPTIMIZE 0x31
#define REG_INVERTIQ 0x33
#define REG_DETECTION_THRESHOLD 0x37
#define REG_SYNC_WORD 0x39
#define REG_INVERTIQ2 0x3b
#define REG_DIO_MAPPING_1 0x40
#define REG_VERSION 0x42
#define REG_PA_DAC 0x4d

// modes
#define MODE_LONG_RANGE_MODE 0x80
#define MODE_SLEEP 0x00
#define MODE_STDBY 0x01
#define MODE_TX 0x03
#define MODE_RX_CONTINUOUS 0x05
#define MODE_RX_SINGLE 0x06

// PA config
#define PA_BOOST 0x80

// IRQ masks
#define IRQ_TX_DONE_MASK 0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK 0x40

#define RF_MID_BAND_THRESHOLD 525E6
#define RSSI_OFFSET_HF_PORT 157
#define RSSI_OFFSET_LF_PORT 164

#define MAX_PKT_LENGTH 255

class SX1278
{
public:
   SX1278(SPI_HandleTypeDef *spi, GPIO_TypeDef *portSS, uint16_t pinSS);
   SX1278(SPI_HandleTypeDef *spi, GPIO_TypeDef *portSS, uint16_t pinSS, GPIO_TypeDef *portRST, uint16_t pinRST);
   ~SX1278();

private:
   uint8_t readReg(uint8_t reg);
   void writeReg(uint8_t reg, uint8_t value);
   void hwReset();
   void setMode(uint8_t mode);
   void setTxPower(uint8_t level);
   void setFrequency(long frequency);

   void setSpreadingFactor(int sf);
   int getSpreadingFactor();

   void setSignalBandwidth(long sbw);
   long getSignalBandwidth();
   void setLdoFlag();

   bool isTransmitting();

   void explicitHeaderMode();
   void implicitHeaderMode();

   void disableInvertIQ();
   void enableInvertIQ();

   void disableCrc();
   void enableCrc();

   void setSyncWord(int sw);
   void setPreambleLength(long length);

   void setGain(uint8_t gain);
   byte random();
   void setOCP(uint8_t mA);

   void setCodingRate4(int denominator);

   int beginPacket(int implicitHeader);
   int endPacket();
   int parsePacket(int size);
   int read();
   int available();
   void receive(int size);

   int peek();
   int rssi();
   int packetRssi();
   float packetSnr();

   long packetFrequencyError();

   pinConfig();
   bool init();
   void setSS(bool state);

   SPI_HandleTypeDef *_spi;

   GPIO_TypeDef *_portSS;
   uint16_t _pinSS;

   GPIO_TypeDef *_portRST;
   uint16_t _pinRST;
   bool resetPinDefined = false;
   int _implicitHeaderMode = 0;
   long _frequency = 433E6;
   int _packetIndex = 0;
};