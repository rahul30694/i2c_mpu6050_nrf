extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #include "utility/nrftwi.h"
}

#include "Wire.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxBufferIndex = 0;
uint8_t TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txBufferIndex = 0;
uint8_t TwoWire::txBufferLength = 0;

uint8_t TwoWire::transmitting = 0;
void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire()
{
}

void TwoWire::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  twi_master_init();
}

void TwoWire::begin(uint8_t address)
{
  twi_master_transfer(uint8_t address,uint8_t *data,uint8_t data_length,bool issue_stop_condition)

}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, bool issue_stop_condition)
{
  bool transfer_success=false;
 if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
   }
 
  if((address & TWI_READ_BIT))
  {
   transfer_success = twi_master_read(data,data_length,issue_stop_condition);
  }
  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = data_length;

return transfer_success;  

}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}


void TwoWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}


uint8_t TwoWire::endTransmission(bool issue_stop_condition)
{
  // transmit buffer (blocking)
  int8_t ret = twi_master_write(data,data_length,issue_stop_condition);
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  
  transmitting = 0;
  return ret;
}


size_t TwoWire::write(uint8_t data)
{
     if(txBufferLength >= BUFFER_LENGTH){
      
      return 0;
}
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
    return 1;
}

int TwoWire::read(void)
{
  int value = -1;
  
  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}


int TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

