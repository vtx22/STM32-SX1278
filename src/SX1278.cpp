#include "SX1278.hpp"

#ifdef PRINTER_DEBUG
extern PRINTER printer;
#endif

SX1278::SX1278(SPI_HandleTypeDef *spi, GPIO_TypeDef *portSS, uint16_t pinSS) : _spi(spi), _portSS(portSS), _pinSS(pinSS)
{
   pinConfig();
   __HAL_SPI_ENABLE(_spi);
   init();
}

SX1278::SX1278(SPI_HandleTypeDef *spi, GPIO_TypeDef *portSS, uint16_t pinSS, GPIO_TypeDef *portRST, uint16_t pinRST) : _spi(spi), _portSS(portSS), _pinSS(pinSS), _portRST(portRST), _pinRST(pinRST)
{
   resetPinDefined = true;
   pinConfig();
   __HAL_SPI_ENABLE(_spi);
   init();
}

void SX1278::setDIO0(GPIO_TypeDef *portDIO0, uint16_t pinDIO0)
{
   _portDIO0 = portDIO0;
   _pinDIO0 = pinDIO0;

   GPIO_InitTypeDef GPIO_InitStruct = {0};

   GPIO_InitStruct.Pin = _pinDIO0;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(_portDIO0, &GPIO_InitStruct);

   useDIO0 = true;
}

SX1278::~SX1278()
{
   setMode(MODE_SLEEP);
}

bool SX1278::init()
{
#ifdef PRINTER_DEBUG
   printer.print("SX1278 Init...\n");
#endif
   hwReset();

   uint8_t version = readReg(REG_VERSION);
#ifdef PRINTER_DEBUG
   printer.print("Version is:");
   printer.print(version);
#endif
   if (version != 0x12)
   {
      return false;
   }

   // put in sleep mode
   setMode(MODE_SLEEP);

   // Set Gain to max
   setGain(6);

   // set frequency
   setFrequency(_frequency);

   // set base addresses
   writeReg(REG_FIFO_TX_BASE_ADDR, 0);
   writeReg(REG_FIFO_RX_BASE_ADDR, 0);

   // set LNA boost
   writeReg(REG_LNA, readReg(REG_LNA) | 0x03);

   // set auto AGC
   writeReg(REG_MODEM_CONFIG_3, 0x04);

   // set output power
   setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);

   // put in standby mode
   setMode(MODE_STDBY);
#ifdef PRINTER_DEBUG
   printer.print("\nSX1278 Standby! Init Complete!\n");
#endif
   return true;
}

void SX1278::setMode(uint8_t mode)
{
   writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | mode);
}

void SX1278::setTxPower(uint8_t level, int outputPin)
{
   if (PA_OUTPUT_RFO_PIN == outputPin)
   {
      // RFO
      if (level < 0)
      {
         level = 0;
      }
      else if (level > 14)
      {
         level = 14;
      }

      writeReg(REG_PA_CONFIG, 0x70 | level);
   }
   else
   {
      // PA BOOST
      if (level > 17)
      {
         if (level > 20)
         {
            level = 20;
         }

         // subtract 3 from level, so 18 - 20 maps to 15 - 17
         level -= 3;

         // High Power +20 dBm Operation (Semtech SX1276/77/78/79 5.4.3.)
         writeReg(REG_PA_DAC, 0x87);
         setOCP(140);
      }
      else
      {
         if (level < 2)
         {
            level = 2;
         }
         // Default value PA_HF/LF or +17dBm
         writeReg(REG_PA_DAC, 0x84);
         setOCP(100);
      }

      writeReg(REG_PA_CONFIG, PA_BOOST | (level - 2));
   }
}

void SX1278::setSignalBandwidth(long sbw)
{
   int bw;

   if (sbw <= 7.8E3)
   {
      bw = 0;
   }
   else if (sbw <= 10.4E3)
   {
      bw = 1;
   }
   else if (sbw <= 15.6E3)
   {
      bw = 2;
   }
   else if (sbw <= 20.8E3)
   {
      bw = 3;
   }
   else if (sbw <= 31.25E3)
   {
      bw = 4;
   }
   else if (sbw <= 41.7E3)
   {
      bw = 5;
   }
   else if (sbw <= 62.5E3)
   {
      bw = 6;
   }
   else if (sbw <= 125E3)
   {
      bw = 7;
   }
   else if (sbw <= 250E3)
   {
      bw = 8;
   }
   else /*if (sbw <= 250E3)*/
   {
      bw = 9;
   }

   writeReg(REG_MODEM_CONFIG_1, (readReg(REG_MODEM_CONFIG_1) & 0x0f) | (bw << 4));
   setLdoFlag();
}

size_t SX1278::write(const uint8_t *buffer, size_t size)
{
   int currentLength = readReg(REG_PAYLOAD_LENGTH);

   // check size
   if ((currentLength + size) > MAX_PKT_LENGTH)
   {
      size = MAX_PKT_LENGTH - currentLength;
   }

   // write data
   for (size_t i = 0; i < size; i++)
   {
      writeReg(REG_FIFO, buffer[i]);
   }

   // update length
   writeReg(REG_PAYLOAD_LENGTH, currentLength + size);

   return size;
}

size_t SX1278::write(std::vector<uint8_t> data)
{
   return write(&data[0], data.size());
}

size_t SX1278::write(std::vector<char> data)
{
   return write(&data[0], data.size());
}

size_t SX1278::write(const char *buffer, size_t size)
{
   return write((uint8_t *)buffer, size);
}

long SX1278::getSignalBandwidth()
{
   uint8_t bw = (readReg(REG_MODEM_CONFIG_1) >> 4);

   switch (bw)
   {
   case 0:
      return 7.8E3;
   case 1:
      return 10.4E3;
   case 2:
      return 15.6E3;
   case 3:
      return 20.8E3;
   case 4:
      return 31.25E3;
   case 5:
      return 41.7E3;
   case 6:
      return 62.5E3;
   case 7:
      return 125E3;
   case 8:
      return 250E3;
   case 9:
      return 500E3;
   }

   return -1;
}

void SX1278::setLdoFlag()
{
   // Section 4.1.1.5
   long symbolDuration = 1000 / (getSignalBandwidth() / (1L << getSpreadingFactor()));

   // Section 4.1.1.6
   bool ldoOn = symbolDuration > 16;

   uint8_t config3 = readReg(REG_MODEM_CONFIG_3);

   config3 = config3 & (ldoOn << 3);

   writeReg(REG_MODEM_CONFIG_3, config3);
}

int SX1278::beginPacket(int implicitHeader)
{
   if (isTransmitting())
   {
      return 0;
   }

   // put in standby mode
   setMode(MODE_STDBY);

   if (implicitHeader)
   {
      implicitHeaderMode();
   }
   else
   {
      explicitHeaderMode();
   }

   // reset FIFO address and paload length
   writeReg(REG_FIFO_ADDR_PTR, 0);
   writeReg(REG_PAYLOAD_LENGTH, 0);

   return 1;
}

int SX1278::endPacket(bool async)
{
	if(async)
	{
		writeReg(REG_DIO_MAPPING_1, 0x40);
		writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
		return 1;
	}

   // put in TX mode
   writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);

   // wait for TX done
   while ((readReg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0)
   {
   }
   // clear IRQ's
   writeReg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);

   return 1;
}

bool SX1278::isTransmitting()
{
   if ((readReg(REG_OP_MODE) & MODE_TX) == MODE_TX)
   {
      return true;
   }

   if (readReg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK)
   {
      // clear IRQ's
      writeReg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
   }

   return false;
}

int SX1278::parsePacket(int size)
{
   int packetLength = 0;
   int irqFlags = readReg(REG_IRQ_FLAGS);

   if (size > 0)
   {
      implicitHeaderMode();

      writeReg(REG_PAYLOAD_LENGTH, size & 0xff);
   }
   else
   {
      explicitHeaderMode();
   }

   // clear IRQ's
   writeReg(REG_IRQ_FLAGS, irqFlags);

   if ((irqFlags & IRQ_RX_DONE_MASK) && (irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0)
   {
      // received a packet
      _packetIndex = 0;

      // read packet length
      if (_implicitHeaderMode)
      {
         packetLength = readReg(REG_PAYLOAD_LENGTH);
      }
      else
      {
         packetLength = readReg(REG_RX_NB_BYTES);
      }

      // set FIFO address to current RX address
      writeReg(REG_FIFO_ADDR_PTR, readReg(REG_FIFO_RX_CURRENT_ADDR));

      // put in standby mode
      setMode(MODE_STDBY);
   }
   else if (readReg(REG_OP_MODE) != (MODE_LONG_RANGE_MODE | MODE_RX_SINGLE))
   {
      // not currently in RX mode

      // reset FIFO address
      writeReg(REG_FIFO_ADDR_PTR, 0);

      // put in single RX mode
      writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE);
   }

   return packetLength;
}

int SX1278::available()
{
   return (readReg(REG_RX_NB_BYTES) - _packetIndex);
}

void SX1278::receive(int size)
{

   writeReg(REG_DIO_MAPPING_1, 0x00); // DIO0 => RXDONE

   if (size > 0)
   {
      implicitHeaderMode();

      writeReg(REG_PAYLOAD_LENGTH, size & 0xff);
   }
   else
   {
      explicitHeaderMode();
   }

   writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

void SX1278::setSpreadingFactor(int sf)
{
   if (sf < 6)
   {
      sf = 6;
   }
   else if (sf > 12)
   {
      sf = 12;
   }

   if (sf == 6)
   {
      writeReg(REG_DETECTION_OPTIMIZE, 0xc5);
      writeReg(REG_DETECTION_THRESHOLD, 0x0c);
   }
   else
   {
      writeReg(REG_DETECTION_OPTIMIZE, 0xc3);
      writeReg(REG_DETECTION_THRESHOLD, 0x0a);
   }

   writeReg(REG_MODEM_CONFIG_2, (readReg(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));
   setLdoFlag();
}

void SX1278::setPreambleLength(long length)
{
   writeReg(REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
   writeReg(REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

void SX1278::setSyncWord(int sw)
{
   writeReg(REG_SYNC_WORD, sw);
}

void SX1278::enableCrc()
{
   writeReg(REG_MODEM_CONFIG_2, readReg(REG_MODEM_CONFIG_2) | 0x04);
}

void SX1278::disableCrc()
{
   writeReg(REG_MODEM_CONFIG_2, readReg(REG_MODEM_CONFIG_2) & 0xfb);
}

void SX1278::enableInvertIQ()
{
   writeReg(REG_INVERTIQ, 0x66);
   writeReg(REG_INVERTIQ2, 0x19);
}

void SX1278::disableInvertIQ()
{
   writeReg(REG_INVERTIQ, 0x27);
   writeReg(REG_INVERTIQ2, 0x1d);
}

void SX1278::setOCP(uint8_t mA)
{
   uint8_t ocpTrim = 27;

   if (mA <= 120)
   {
      ocpTrim = (mA - 45) / 5;
   }
   else if (mA <= 240)
   {
      ocpTrim = (mA + 30) / 10;
   }

   writeReg(REG_OCP, 0x20 | (0x1F & ocpTrim));
}

void SX1278::setGain(uint8_t gain)
{
   // check allowed range
   if (gain > 6)
   {
      gain = 6;
   }

   // set to standby
   setMode(MODE_STDBY);

   // set gain
   if (gain == 0)
   {
      // if gain = 0, enable AGC
      writeReg(REG_MODEM_CONFIG_3, 0x04);
   }
   else
   {
      // disable AGC
      writeReg(REG_MODEM_CONFIG_3, 0x00);

      // clear Gain and set LNA boost
      writeReg(REG_LNA, 0x03);

      // set gain
      writeReg(REG_LNA, readReg(REG_LNA) | (gain << 5));
   }
}

uint8_t SX1278::random()
{
   return readReg(REG_RSSI_WIDEBAND);
}

void SX1278::setCodingRate4(int denominator)
{
   if (denominator < 5)
   {
      denominator = 5;
   }
   else if (denominator > 8)
   {
      denominator = 8;
   }

   int cr = denominator - 4;

   writeReg(REG_MODEM_CONFIG_1, (readReg(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
}

int SX1278::peek()
{
   if (!available())
   {
      return -1;
   }

   // store current FIFO address
   int currentAddress = readReg(REG_FIFO_ADDR_PTR);

   // read
   uint8_t b = readReg(REG_FIFO);

   // restore FIFO address
   writeReg(REG_FIFO_ADDR_PTR, currentAddress);

   return b;
}

int SX1278::read()
{
   if (!available())
   {
      return -1;
   }

   _packetIndex++;

   return readReg(REG_FIFO);
}

long SX1278::packetFrequencyError()
{
   int32_t freqError = 0;
   freqError = static_cast<int32_t>(readReg(REG_FREQ_ERROR_MSB) & 0b111);
   freqError <<= 8L;
   freqError += static_cast<int32_t>(readReg(REG_FREQ_ERROR_MID));
   freqError <<= 8L;
   freqError += static_cast<int32_t>(readReg(REG_FREQ_ERROR_LSB));

   if (readReg(REG_FREQ_ERROR_MSB) & 0b1000)
   {                       // Sign bit is on
      freqError -= 524288; // B1000'0000'0000'0000'0000
   }

   const float fXtal = 32E6;                                                                                         // FXOSC: crystal oscillator (XTAL) frequency (2.5. Chip Specification, p. 14)
   const float fError = ((static_cast<float>(freqError) * (1L << 24)) / fXtal) * (getSignalBandwidth() / 500000.0f); // p. 37

   return static_cast<long>(fError);
}

int SX1278::rssi()
{
   return (readReg(REG_RSSI_VALUE) - (_frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

int SX1278::packetRssi()
{
   return (readReg(REG_PKT_RSSI_VALUE) - (_frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

float SX1278::packetSnr()
{
   return ((int8_t)readReg(REG_PKT_SNR_VALUE)) * 0.25;
}

int SX1278::getSpreadingFactor()
{
   return readReg(REG_MODEM_CONFIG_2) >> 4;
}

void SX1278::explicitHeaderMode()
{
   _implicitHeaderMode = 0;

   writeReg(REG_MODEM_CONFIG_1, readReg(REG_MODEM_CONFIG_1) & 0xfe);
}

void SX1278::implicitHeaderMode()
{
   _implicitHeaderMode = 1;

   writeReg(REG_MODEM_CONFIG_1, readReg(REG_MODEM_CONFIG_1) | 0x01);
}

void SX1278::setFrequency(long frequency)
{
   _frequency = frequency;

   uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

   writeReg(REG_FRF_MSB, (uint8_t)(frf >> 16));
   writeReg(REG_FRF_MID, (uint8_t)(frf >> 8));
   writeReg(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

uint8_t SX1278::readReg(uint8_t reg)
{
   uint8_t msg = reg & 0x7F;
   uint8_t value = 0;
   setSS(false);
   HAL_SPI_Transmit(_spi, &msg, 1, 1);
   HAL_SPI_Receive(_spi, &value, 1, 1);
   setSS(true);
   return value;
}

void SX1278::writeReg(uint8_t reg, uint8_t value)
{
   uint8_t msg[2] = {reg | 0x80, value};
   setSS(false);
   HAL_SPI_Transmit(_spi, msg, 2, 1);
   setSS(true);
}

void SX1278::pinConfig()
{
#ifdef PRINTER_DEBUG
   printer.print("SX1278 Pin Config\n");
#endif

   GPIO_InitTypeDef GPIO_InitStruct = {0};

   GPIO_InitStruct.Pin = _pinSS;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

   HAL_GPIO_Init(_portSS, &GPIO_InitStruct);

   if (resetPinDefined)
   {
      GPIO_InitStruct.Pin = _pinRST;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

      HAL_GPIO_Init(_portRST, &GPIO_InitStruct);

      HAL_GPIO_WritePin(_portRST, _pinRST, GPIO_PIN_SET);
   }

   setSS(true);
}

void SX1278::hwReset()
{
#ifdef PRINTER_DEBUG
   printer.print("SX1278 Resetting...\n");
#endif
   if (!resetPinDefined)
   {
      return;
   }
   HAL_GPIO_WritePin(_portRST, _pinRST, GPIO_PIN_RESET);
   HAL_Delay(10);
   HAL_GPIO_WritePin(_portRST, _pinRST, GPIO_PIN_SET);
   HAL_Delay(10);
}

void SX1278::setSS(bool state)
{
   HAL_GPIO_WritePin(_portSS, _pinSS, state ? (GPIO_PIN_SET) : (GPIO_PIN_RESET));
}
