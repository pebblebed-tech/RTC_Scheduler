#include "ext_eeprom_component.h"

namespace esphome {
namespace ext_eeprom_component {

static const char *TAG = "ext_eeprom";

void ExtEepromComponent::setup() {
  if (!this->is_connected(this->address_)) {
    ESP_LOGE(TAG, "Device on address 0x%x not found!", this->address_);
    this->mark_failed(); 
    }
    else
      {
        ESP_LOGE(TAG, "Memory detected!");
      }
}

void  ExtEepromComponent::loop() {

}

void  ExtEepromComponent::dump_config(){
  ESP_LOGCONFIG(TAG, "Ext Eeprom component");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG,"Size = %d",this->get_memory_size());
  ESP_LOGCONFIG(TAG,"Page size = %d",this->get_page_size());
  ESP_LOGCONFIG(TAG,"Page write time = %d",this->get_page_write_time());
  ESP_LOGCONFIG(TAG,"Poll for write complete = %s",(this->get_poll_for_write_complete() ? "true" : "false"));
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with device failed!");
     }
}
/// @brief This checks whether the device is connected and not busy
/// @param Caller can pass in an 0xFF I2C address. This is helpful for larger EEPROMs that have two addresses (see block bit 2). 
/// @return an boolean True for connected
bool ExtEepromComponent::is_connected(uint8_t i2cAddress)
{
  i2c::ErrorCode err;
   if (i2cAddress == 255)
    err = this->bus_->write(i2cAddress, nullptr, 0, true);
  else 
    err = this->bus_->write(this->address_, nullptr, 0, true);
  if (err != i2c::ERROR_OK)
   ESP_LOGE(TAG,"EEPROM not connected and raise this error %d",err);
  return (err == i2c::ERROR_OK);
}
/// @brief This check if the device is busy from the previous request
/// @param Caller can pass in an 0xFF I2C address. This is helpful for larger EEPROMs that have two addresses (see block bit 2).
/// @return False if the device is ready for the next request. True if device is not answering (currently writing)

 bool ExtEepromComponent::is_busy(uint8_t i2cAddress)
{
  if (is_connected(i2cAddress))
    return (false);
  return (true);
} 
/// @brief Reads a byte from a given location
/// @param memaddr is the location to read
/// @return the byte read from device
uint8_t ExtEepromComponent::read8(uint32_t memaddr)
{
  uint8_t tempByte;
  read(memaddr, &tempByte, 1);
  return tempByte;
}
/// @brief Reads a 16 bit word from a given location
/// @param memaddr is the location to read
/// @return the word read from the device
uint16_t ExtEepromComponent::read16(uint32_t memaddr) {
  uint16_t val;
  this->read(memaddr, (uint8_t *) &val, sizeof(uint16_t));
  return val;
}
/// @brief Reads a 32 bit word from a given location
/// @param memaddr is the location to read
/// @return the word read from the device
uint32_t ExtEepromComponent::read32(uint32_t memaddr) {
  uint32_t val;
  this->read(memaddr, (uint8_t *) &val, sizeof(uint32_t));
  return val;
}
/// @brief Reads a float from a given location
/// @param memaddr is the location to read
/// @return the float read from the device
float ExtEepromComponent::read_float(uint32_t memaddr) {
  float val;
  this->read(memaddr, (uint8_t *) &val, sizeof(float));
  return val;
}
/// @brief Reads a double from a given location
/// @param memaddr is the location to read
/// @return the double read from the device
double ExtEepromComponent::read_double(uint32_t memaddr) {
  double val;
  this->read(memaddr, (uint8_t *) &val, sizeof(double));
  return val;
}
/// @brief Bulk read from the device
/// @note breaking up read amt into 32 byte chunks (can be overriden with setI2CBufferSize)
/// @note Handles a read that straddles the 512kbit barrier
/// @param memaddr is the starting location to read
/// @param buff is the pointer to an array of bytes that will be used to store the data received
/// @param bufferSize is the size of the buffer and also the number of bytes to be read
void ExtEepromComponent::read(uint32_t memaddr, uint8_t *buff, uint16_t bufferSize)
{
  ESP_LOGVV(TAG,"Read %d bytes from address %d",bufferSize,memaddr);
  uint32_t size = bufferSize;
  uint8_t *p = buff;
  while (size >= 1) {
    //Limit the amount to read to a page size
    uint16_t amtToRead = size;
    if (amtToRead > I2C_BUFFERSIZE) //I2C buffer size limit
      amtToRead = I2C_BUFFERSIZE;

    //Check if we are dealing with large (>512kbit) EEPROMs
    uint8_t i2cAddress = this->address_;
    if (this->get_memory_size() > 0xFFFF)
    {
      //Figure out if we are going to cross the barrier with this read
      if (memaddr < 0xFFFF)
      {
        if (0xFFFF - memaddr < amtToRead) //0xFFFF - 0xFFFA < I2C_BUFFERSIZE
          amtToRead = 0xFFFF - memaddr;   //Limit the read amt to go right up to edge of barrier
      }

      //Figure out if we are accessing the lower half or the upper half
      if (memaddr > 0xFFFF)
        i2cAddress |= 0b100; //Set the block bit to 1
    }

    //See if EEPROM is available or still writing a previous request
    while (this->poll_for_write_complete_ && is_busy(i2cAddress) == true) //Poll device
      delayMicroseconds(100);          //This shortens the amount of time waiting between writes but hammers the I2C bus
    ESP_LOGVV(TAG,"Read - Done Poll");

    i2c::ErrorCode ret;
    if(this->get_memory_size() > 2048){
      uint8_t maddr[] = {(uint8_t) (memaddr >> 8), (uint8_t) (memaddr & 0xFF)};
      ret = this->bus_->write(this->address_, maddr, 2, false);
    }
    else
    {
      uint8_t maddr[] = { (uint8_t) (memaddr & 0xFF)};
      ret = this->bus_->write(this->address_, maddr, 1, false);
    }
    if (ret != i2c::ERROR_OK) 
    {
      ESP_LOGE(TAG,"Read raise this error %d on setting address",ret);
    }
    ESP_LOGVV(TAG,"Read - Done Set address, Ammount to read %d",amtToRead);
    ret = this->bus_->read(this->address_, p, amtToRead);
    if (ret != i2c::ERROR_OK) 
    {
      ESP_LOGE(TAG,"Read raised this error %d on reading data",ret);
    }
    ESP_LOGVV(TAG,"Done Read");
    memaddr += amtToRead;
    p += amtToRead;
    size -= amtToRead;
  } 
}
/// @brief Read a std::string from the device
/// @note It write the string with an extra byte containing the size at the memaddr
/// @note It is limited to reading a max length of 254 bytes
/// @param memaddr is the starting location to read
/// @param strToRead will hold the bytes read from the device on return of the fuction

uint32_t ExtEepromComponent::readStringFromEEPROM(uint32_t memaddr, std::string& strToRead)
{
  uint8_t newStrLen = read8(memaddr);
  uint8_t data[newStrLen + 1];
  read(memaddr+1,(uint8_t *)data,newStrLen);
  data[newStrLen] = '\0'; 
  strToRead = (char *) data;
  return memaddr + 1 + newStrLen;
}
/// @brief Writes a byte to a given location
/// @note It will check first to see if loccation already has the value to protect write cycles
/// @param memaddr is the location to write
/// @param dataToWrite contains the byte to be written
void ExtEepromComponent::write8(uint32_t memaddr, uint8_t dataToWrite)
{
   if (read8(memaddr) != dataToWrite) //Update only if data is new
    write(memaddr, &dataToWrite, 1);
    
}
/// @brief Writes a 16 bit word to a given location
/// @note It will check first to see if loccation already has the value to protect write cycles
/// @param memaddr is the location to write
/// @param value contains the word to be written
void ExtEepromComponent::write16(uint32_t memaddr, uint16_t value) {
  if (read16(memaddr) != value) //Update only if data is new
  {
    uint16_t val = value;
    this->write(memaddr, (uint8_t *) &val, sizeof(uint16_t));
  }
}
/// @brief Writes a 32 bit word to a given location
/// @note It will check first to see if loccation already has the value to protect write cycles
/// @param memaddr is the location to write
/// @param value contains the word to be written
void ExtEepromComponent::write32(uint32_t memaddr, uint32_t value) {
  if (read32(memaddr) != value) //Update only if data is new
  {
    uint32_t val = value;
    this->write(memaddr, (uint8_t *) &val, sizeof(uint32_t));
  }
}
/// @brief Writes a float to a given location
/// @note It will check first to see if loccation already has the value to protect write cycles
/// @param memaddr is the location to write
/// @param value contains the float to be written
void ExtEepromComponent::write_float(uint32_t memaddr, float value) {
  if (read_float(memaddr) != value) //Update only if data is new
  {
    float val = value;
    this->write(memaddr, (uint8_t *) &val, sizeof(float));
  }
}
/// @brief Writes a double to a given location
/// @note It will check first to see if loccation already has the value to protect write cycles
/// @param memaddr is the location to write
/// @param value contains the double to be written
void ExtEepromComponent::write_double(uint32_t memaddr, double value) {
  if (read_double(memaddr) != value) //Update only if data is new
  {
    double val = value;
    this->write(memaddr, (uint8_t *) &val, sizeof(double));
  }
}
/// @brief Bulk write to the device
/// @note breaking up read amt into 32 byte chunks (can be overriden with setI2CBufferSize)
/// @note Handles a write that straddles the 512kbit barrier
/// @param memaddr is the starting location to write
/// @param dataToWrite is the pointer to an array of bytes that will be written
/// @param bufferSize is the size of the buffer and also the number of bytes to be written
void ExtEepromComponent::write(uint32_t memaddr,  uint8_t *dataToWrite, uint16_t bufferSize)
{
  ESP_LOGVV(TAG,"Write %d bytes to address %d",bufferSize,memaddr);
  //uint32_t memaddr = memaddr;
  uint32_t size = bufferSize;
  uint8_t *p = dataToWrite;
   // Check to make sure write is inside device range
  if (memaddr + bufferSize >= this->memory_size_bytes_)
  {
    bufferSize = this->memory_size_bytes_ - memaddr; // if not shorten the write to fit
    ESP_LOGE(TAG,"Trying write data beyond device size, Address %d", (memaddr + bufferSize));
  }

  uint16_t maxWriteSize =  this->memory_page_size_bytes_;
  if (maxWriteSize > I2C_BUFFERSIZE)
    maxWriteSize = I2C_BUFFERSIZE; 

  //Break the buffer into page sized chunks

  while (size >= 1)
  {
    //Limit the amount to write to either the page size or the I2C limit
    uint16_t amtToWrite = size;
    if (amtToWrite > maxWriteSize)
      amtToWrite = maxWriteSize;

    if (amtToWrite > 1)
    {
      //Check for crossing of a page line. Writes cannot cross a page line.
      uint16_t pageNumber1 = memaddr / this->memory_page_size_bytes_;
      uint16_t pageNumber2 = (memaddr + amtToWrite - 1) / this->memory_page_size_bytes_;
      if (pageNumber2 > pageNumber1)
        amtToWrite = (pageNumber2 * this->memory_page_size_bytes_) - memaddr; //Limit the write amt to go right up to edge of page barrier
    }
    /// Check if we are dealing with large (>512kbit) EEPROMs
    uint8_t i2cAddress = this->address_;
    if (this->get_memory_size() > 0xFFFF)
    {
      /// Figure out if we are going to cross the barrier with this write
      if (memaddr < 0xFFFF)
      {
        if (0xFFFF - memaddr < amtToWrite) /// 0xFFFF - 0xFFFA < I2C_BUFFERSIZE
          amtToWrite = 0xFFFF - memaddr;   /// Limit the write amt to go right up to edge of barrier
      }

      /// Figure out if we are accessing the lower half or the upper half
      if (memaddr > 0xFFFF)
        i2cAddress |= 0b100; /// Set the block bit to 1
    }

    /// See if device is available or still writing a previous request
    while (this->poll_for_write_complete_ && is_busy(i2cAddress) == true) //Poll device
      delayMicroseconds(100);          /// This shortens the amount of time waiting between writes but hammers the I2C bus
    ESP_LOGVV(TAG,"Done Poll");
    ESP_LOGVV(TAG,"Write block %d bytes to address %d",amtToWrite,memaddr);
    this->write_block_(memaddr, p, amtToWrite);
    memaddr += amtToWrite;
    p += amtToWrite;
    size -= amtToWrite;
    ESP_LOGVV(TAG,"After write size %d amt  %d add %d",size, amtToWrite,memaddr);
    if (this->get_poll_for_write_complete() == false)
      delay(this->memory_page_write_time_ms_); /// Delay the amount of time to record a page
  }
}

/// @brief Write a std::string to the device
/// @note It writes the string with an extra byte containing the size at the memaddr eg address 0
/// @note it is limited to writing a max length of the string of 254 bytes and will trim extra bytes
/// @param memaddr is the starting location to write
/// @param strToWrite contains the std::string to be wriiten
uint32_t ExtEepromComponent::writeStringToEEPROM(uint32_t memaddr, std::string &strToWrite)
{
  if (strToWrite.length()> 254){
    ESP_LOGE(TAG,"String to long. Limit is 254 chars" );
    strToWrite.resize(254);
  }
  uint8_t len = strToWrite.length();
  const char *p = strToWrite.c_str();
  write8(memaddr, len);
  write(memaddr+1, (uint8_t *)p, len);
  return memaddr + 1 + len;
}
void ExtEepromComponent::dump_EEPROM(uint32_t start_addr,uint16_t word_count ){
  std::vector<uint16_t> words;
  uint16_t address;
  uint16_t data;
  std::string res;
  char adbuf[8];
  char buf[5];
  size_t len;
  address = start_addr;
  while (address < (word_count + start_addr))
  {
    for (size_t i = address; i < (address+16); i+=2)
    {
      this->read_object(i, data);
      words.push_back(data);
    }
    sprintf(adbuf, "%04X : ", address);
    res = adbuf;
    len = words.size();
    for (size_t u = 0; u < len; u++) {
      if (u > 0) {
        res += " ";
      }
      sprintf(buf, "%04X", words[u]);
      res += buf;
    }
    ESP_LOGD(TAG, "%s", res.c_str()); 
    words.clear();
    address = address +16; 
  }
}

/// @brief Erase the entire device
/// @note **** to be used carefully, as there is no recovery ****
/// @param toWrite optional value to be written to all locations defaults to 0x00
void ExtEepromComponent::erase(uint8_t toWrite)
{
  uint8_t tempBuffer[this->memory_page_size_bytes_];
  for (uint32_t x = 0; x < this->memory_page_size_bytes_; x++)
    tempBuffer[x] = toWrite;

  for (uint32_t addr = 0; addr < this->get_memory_size(); addr += this->memory_page_size_bytes_)
    write(addr, tempBuffer, this->memory_page_size_bytes_);
}
/// @brief Sets the size of the device in bytes
/// @param memSize contains the size of the device
void ExtEepromComponent::set_memory_size(uint32_t memSize)
{
  memory_size_bytes_ = memSize;
}
/// @brief Gets the user specified size of the device in bytes
/// @return size in bytes
uint32_t ExtEepromComponent::get_memory_size()
{
  return memory_size_bytes_;
}
/// @brief Sets the page size of the device in bytes
/// @param pageSize contains the size of the device pages
void ExtEepromComponent::set_page_size(uint8_t pageSize)
{
  memory_page_size_bytes_ = pageSize;
}
/// @brief Gets the user specified size of the device pages in bytes
/// @return Page size in bytes
uint8_t ExtEepromComponent::get_page_size()
{
  return memory_page_size_bytes_;
}
/// @brief Sets the page write for the device in ms
/// @param writeTimeMS contains the time to write a page of the device
void ExtEepromComponent:: set_page_write_time(uint8_t writeTimeMS)
{
   memory_page_write_time_ms_ = writeTimeMS;
}
/// @brief Gets the user specified write time for a device page in ms
/// @return page write time in ms
uint8_t ExtEepromComponent:: get_page_write_time()
{
  return  memory_page_write_time_ms_;
}
/// @brief Sets whether the write functions poll for write complete or just wait for set page write time
/// @param pollWriteComplete bool 
void ExtEepromComponent::set_poll_for_write_complete(bool pollWriteComplete)
{
   poll_for_write_complete_ = pollWriteComplete;
}
/// @brief Gets whether user has selected polling or waiting
/// @return true for polling otherwise will wait for the specified page_write_time
bool ExtEepromComponent::get_poll_for_write_complete()
{
  return  poll_for_write_complete_;
} 
// private functions

void ExtEepromComponent::write_block_(uint32_t memaddr, const uint8_t *obj, uint8_t size) {
  
  i2c::WriteBuffer buff[2];
  i2c::ErrorCode ret;
  // Check if the device has two address bytes
  if(this->get_memory_size() > 2048){
      uint8_t maddr[] = {(uint8_t) (memaddr >> 8), (uint8_t) (memaddr & 0xFF)};
      buff[0].data = maddr;
      buff[0].len = 2;
      buff[1].data = obj;
      buff[1].len = size;
      ret = this->bus_->writev(this->address_, buff, 2, true);
    }
    else
    {
      uint8_t maddr[] = { (uint8_t) (memaddr & 0xFF)};
      buff[0].data = maddr;
      buff[0].len = 1;
      buff[1].data = obj;
      buff[1].len = size;
      ret = this->bus_->writev(this->address_, buff, 2, true);
    }
  if (ret != i2c::ERROR_OK) 
    {
      ESP_LOGE(TAG,"Write raise this error %d on writing data to this address %d",ret,memaddr );
    }
}
void ExtEepromComponent:: test(){
  ESP_LOGD(TAG, "Mem size in bytes: %d",this->get_memory_size());
  int32_t myValue2 = -460;
  this->write_object(10, myValue2); //(location, data)
  int32_t myRead2;
  this->read_object(10, myRead2); //location to read, thing to put data into
  ESP_LOGD(TAG, "I read: %d",myRead2 );
  float myValue3 = -7.35;
  this->write_object(20, myValue3); //(location, data)
  float myRead3;
  this->read_object(20, myRead3); //location to read, thing to put data into
  ESP_LOGD(TAG,"I read: %f",myRead3);
  std::string myString = "This is a test ";
  this->writeStringToEEPROM(40, myString); //(location, data)
  std::string myRead4;
  this->readStringFromEEPROM(40, myRead4); //location to read, thing to put data into
  ESP_LOGD(TAG,"I read: %s",myRead4.c_str());
  this->dump_EEPROM(0,96);
}



}  // namespace ext_eeprom_component
}  // namespace esphom