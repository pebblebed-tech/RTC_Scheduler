#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "Arduino.h"
#include "Wire.h"
namespace esphome {
namespace ext_eeprom_component {
#define I2C_BUFFER_LENGTH_RX I2C_BUFFER_LENGTH
#define I2C_BUFFER_LENGTH_TX I2C_BUFFER_LENGTH

struct struct_memorySettings
{
  TwoWire *i2cPort;
  uint8_t deviceAddress;
  uint32_t memorySize_bytes;
  uint16_t pageSize_bytes;
  uint8_t pageWriteTime_ms;
  bool pollForWriteComplete;
  uint16_t i2cBufferSize;
};

class ExtEepromComponent : public i2c::I2CDevice, public Component {

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void test();
  uint8_t read(uint32_t eepromLocation);
  void read(uint32_t eepromLocation, uint8_t *buff, uint16_t bufferSize);
  void write(uint32_t eepromLocation, uint8_t dataToWrite);
  void write(uint32_t eepromLocation, const uint8_t *dataToWrite, uint16_t blockSize);

  bool begin(uint8_t deviceAddress = 0b01010111, TwoWire &wirePort = Wire); //By default use the Wire port
  bool isConnected(uint8_t i2cAddress = 255);
  bool isBusy(uint8_t i2cAddress = 255);
  void erase(uint8_t toWrite = 0x00); //Erase the entire memory. Optional: write a given byte to each spot.
  int readStringFromEEPROM(int addrOffset, String *strToRead);
  int writeStringToEEPROM(int addrOffset, const String &strToWrite);
  //void settings(struct_memorySettings newSettings); //Set all the settings using the settings struct
  void setMemorySize(uint32_t memSize); //Set the size of memory in bytes
  uint32_t getMemorySize();             //Return size of EEPROM
  uint32_t length();                    //Return size of EEPROM
  void setPageSize(uint16_t pageSize);  //Set the size of the page we can write at a time
  uint16_t getPageSize();
  void setPageWriteTime(uint8_t writeTimeMS); //Set the number of ms required per page write
  uint8_t getPageWriteTime();
  void setPollForWriteComplete(bool pollWriteComplete);
  void enablePollForWriteComplete(); //Most EEPROMs all I2C polling of when a write has completed
  void disablePollForWriteComplete();
  uint16_t getI2CBufferSize(); //Return the size of the TX buffer

  //Functionality to 'get' and 'put' objects to and from EEPROM.
  template <typename T>
  T &get(uint32_t idx, T &t)
  {
    uint8_t *ptr = (uint8_t *)&t;
    read(idx, ptr, sizeof(T)); //Address, data, sizeOfData
    return t;
  }

  template <typename T>
  const T &put(uint32_t idx, const T &t) //Address, data
  {
    const uint8_t *ptr = (const uint8_t *)&t;
    write(idx, ptr, sizeof(T)); //Address, data, sizeOfData
    return t;
  }
private:
 
     //Variables
  struct_memorySettings settings = {
      .i2cPort = &Wire,
      .deviceAddress =  0b01010111, //0b1010 + (A2 A1 A0) or 0b1010 + (B0 A1 A0) for larger (>512kbit) EEPROMs
      .memorySize_bytes = 4096,
      .pageSize_bytes = 32,
      .pageWriteTime_ms = 5,
      .pollForWriteComplete = true,
  };
};


}  // namespace empty_i2c_component
}  // namespace esphome