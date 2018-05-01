/*

  ModbusMaster_STM32_RS-485.ino - example using ModbusMaster library for STM32F103C8T6

  Library:: ModbusMaster
  Author:: Doc Walker <4-20ma@wvfans.net>
  Edit::   Egor Orlenok <messaf@mail.ru> (01.2017)

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include "ModbusMaster.h"

/*!
  We're using a MAX485-compatible RS485 Transceiver.
  Rx/Tx is hooked up to the hardware serial port at 'Serial'.
  The slave_data Enable and Receiver Enable pins are hooked up as follows:
*/
//=============== Modbus ================================================//
#define MAX485_TX_Enable      PA1 // Pin for MAX485 Data Direction Control

#define BROADCAST_ADDR        0   // The broadcast address to all slaves
#define MIN_SLAVE_DEVICES     1   // The minimum number of slaves on the bus
#define MAX_SLAVE_DEVICES     128 // Maximum number of slaves on the bus

// For example ID's slaves
#define SlaveID_Arduino_UNO   3
#define SlaveID_Arduino_NANO  127

// Initialization LED on pin PC13 (Blue Pill)
#define LedInit() do{\
    pinMode(LED_BUILTIN, OUTPUT);\
    digitalWrite(LED_BUILTIN, HIGH);\
  }while(0)
#define LedON() digitalWrite(LED_BUILTIN, LOW);
#define LedOFF() digitalWrite(LED_BUILTIN, HIGH);

enum // The names of the slave registers to read/write
{
  // Slave system debug registers
  SLAVE_INFO,             // reg addr: 0
  SLAVE_DEBUG_REG,        // reg addr: 1
  // ADC's channels on Arduino board
  ADC_0,                  // reg addr: 2
  ADC_1,                  // reg addr: 3
  ADC_2,                  // reg addr: 4
  ADC_3,                  // reg addr: 5
  ADC_4,                  // reg addr: 6
  ADC_5,                  // reg addr: 7
  // Loads (Relay)
  LOADS,                  // reg addr: 8
  // Slave counter errors
  SLAVE_ERRORS,           // reg addr: 9
  // Total regs data
  TOTAL_SIZE_SLAVE_DATA   // Total regs addr: 10
};

// Structure to store information about slave data
typedef struct
{
  //============System regs==========//
  uint8_t id;
  uint16_t serial_number;
  uint16_t debug_reg;
  //===========Counters errors=======//
  uint16_t err_IllegalFunction;
  uint16_t err_IllegalDataAddress;
  uint16_t err_IllegalDataValue;
  uint16_t err_SlaveDeviceFailure;
  uint16_t err_InvalidSlaveID;
  uint16_t err_InvalidFunction;
  uint16_t err_ResponseTimedOut;
  uint16_t err_InvalidCRC;
} slaveStruct_t;

//=======================================================================//
// Action before transfer
void preTransmission()
{
  digitalWrite(MAX485_TX_Enable, HIGH);
}
// Action after the transfer is complete
void postTransmission()
{
  digitalWrite(MAX485_TX_Enable, LOW);
}

// idle callback function; gets called during idle time between TX and RX
void IDle(void) {

}

// instantiate ModbusMaster object
ModbusMaster node;

//================== Usessr functions Prototypes===========================//
uint8_t ScanSlaveDevice(uint8_t start_id, uint8_t finish_id, uint8_t timeoutReq);
void getHoldingRegs(uint8_t id, uint16_t timeout, uint8_t retries);
void pollFoundsSlaves(uint8_t slaveFounds);
bool debStruct(uint8_t _id, uint8_t found_slaves);
void error_processing(uint8_t err_code, uint8_t _id);
//=======================================================================//

slaveStruct_t slave_sys_info[MAX_SLAVE_DEVICES];
uint8_t foundSlaves = 0;

void setup()
{
  // Init pin for control MAX485 in receive mode
  pinMode(MAX485_TX_Enable, OUTPUT);
  digitalWrite(MAX485_TX_Enable, LOW);

  LedInit();

  Serial.begin(115200); // USB Serial for debug
  delay(5000); // Wait init slaves (Arduino)
  Serial.println("===================================================");
  Serial.println("----------------- !!! WELCOME !!! -----------------");
  Serial.println("--------- STM32F103C8T6 Modbus RTU MASTER ---------");
  Serial.println("===================================================");

  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  node.idle(IDle);

  // Starts scanning the slave on the bus
  foundSlaves = ScanSlaveDevice(MIN_SLAVE_DEVICES, MAX_SLAVE_DEVICES, 50);

  if (foundSlaves) { // If you found at least one slave device, turn on the LED
    LedON();
  } else {           // If no slaves, flashing LED
    while (1) {
      LedON();  delay(500);
      LedOFF(); delay(500);
    }
  }

  // communicate with Modbus slave ID 1 over Serial (port 2), baud rate
  node.begin(MIN_SLAVE_DEVICES, STM32_USART2, 9600);
  // Initialize the timeout for failing to address to the slave, ms
  // You must set the timeout is not less than zero (0)
  node.setResponseTimeout(200);
}

void loop()
{
  // Appeal to a particular slave
  //  getHoldingRegs(SlaveID_Arduino_UNO);
  //  getHoldingRegs(SlaveID_Arduino_NANO);

  // The poll found the slave
  pollFoundsSlaves(foundSlaves);

}
//=============================================================================//
// -------Examples of of custom functions: ------------------------------------//
/* Scanning the connected slaves:
 *  start_id    - Address to which you want to start scanning
 *  finish_id   - End address scan
 *  timeoutReq  - Pause between the polling address
 *  return      - Returns the number of slaves found
 */
uint8_t ScanSlaveDevice(uint8_t start_id, uint8_t finish_id, uint8_t timeoutReq)
{
  uint16_t temp_timeout = node.getResponseTimeout();
  uint8_t found_slaves = 0;

  node.begin(MIN_SLAVE_DEVICES, STM32_USART2, 9600);
  node.setResponseTimeout(100); // milliseconds

  Serial.println("===================================================");
  Serial.println("Start scaning...");

  for (uint8_t _slaveID = start_id; _slaveID <= finish_id; _slaveID++)
  {
    uint16_t slave_data[2] = {0};
    // Slave ID can be changed "on the fly"
    node.setSlaveID(_slaveID);
    uint8_t result = node.readHoldingRegisters(0, 2);
    if (result == node.ku8MBSuccess)
    {
      found_slaves++;
      slave_sys_info[found_slaves].id = _slaveID;
      slave_sys_info[found_slaves].serial_number = node.getResponseBuffer(SLAVE_INFO);
      slave_sys_info[found_slaves].debug_reg = node.getResponseBuffer(SLAVE_DEBUG_REG);
      Serial.println("---------------------------------------------------");
      Serial.print("Found System info from Slave ID: "); Serial.println(_slaveID);
      Serial.println("---------------------------------------------------");
      Serial.print("Struct ID: "); Serial.println(slave_sys_info[found_slaves].id);
      Serial.print("Register["); Serial.print(SLAVE_INFO); Serial.print("] => ");
      Serial.println(slave_sys_info[found_slaves].serial_number);
      Serial.print("Register["); Serial.print(SLAVE_DEBUG_REG); Serial.print("] => ");
      Serial.println(slave_sys_info[found_slaves].debug_reg);
      Serial.println("---------------------------------------------------");
      LedON(); delay(timeoutReq / 2);
      LedOFF(); delay(timeoutReq / 2);
    }
  }
  Serial.println("---------------------------------------------------");
  Serial.print("Found: "); Serial.println(found_slaves);
  Serial.println("===================================================");

  //Finish operation UART2
  node.setResponseTimeout(temp_timeout);
  node.end(STM32_USART2);

  return found_slaves;
}
//-----------------------------------------------------------------------------//
/* Read Holding Registers:
 *  id      - Slave address, to which the handling will be carried
 *  timeout - Delay after handling
 *  retries - The number of of repeated requests, if the previous request was fail
 */
void getHoldingRegs(uint8_t id, uint16_t timeout, uint8_t retries)
{
  uint16_t slave_data[TOTAL_SIZE_SLAVE_DATA] = {0};
  uint8_t retry_cnt = 0;
  uint8_t state = 0;

  // Slave ID can be changed "on the fly"
  node.setSlaveID(id);

  do {
    // slave: read (TOTAL_SIZE_SLAVE_DATA) 16-bit registers starting at register 0 to RX buffer
    state = node.readHoldingRegisters(0, TOTAL_SIZE_SLAVE_DATA);
    error_processing(state, id);
  } while (state && (++retry_cnt < retries));

  Serial.println("===================================================");
  Serial.print("Slave ID: ");  Serial.println(node.getSlaveID());
  Serial.println("---------------------------------------------------");
  // do something with slave_data if read is successful
  if (state == node.ku8MBSuccess)
  {
    for (uint8_t reg_num = 0; reg_num < TOTAL_SIZE_SLAVE_DATA; reg_num++)
    {
      slave_data[reg_num] = node.getResponseBuffer(reg_num);
      Serial.print("Slave register["); Serial.print(reg_num); Serial.print("] => ");
      Serial.println(slave_data[reg_num]);
    }
  } else {
    Serial.println("---------------------------------------------------");
    Serial.print("Error code: 0x"); Serial.println(state, HEX);
  }
  Serial.print("Retries: "); Serial.println(retry_cnt);
  Serial.println("===================================================");
  // After each communicating with the the slave needs to be cleaned receive and transmit buffers
  node.clearResponseBuffer();
  node.clearTransmitBuffer();
  //-------------------------------------------------------------------------------//
  delay(timeout); // Between the calls to the various slave desirable to pause
  //-------------------------------------------------------------------------------//
}
//-----------------------------------------------------------------------------//
/* The poll found the slave:
 *  slaveFounds - Number of slaves found
 */
void pollFoundsSlaves(uint8_t slaveFounds)
{
  for (uint8_t i = MIN_SLAVE_DEVICES; i < slaveFounds + 1; i++)
  {
    getHoldingRegs(slave_sys_info[i].id, 50, 3);
  }
}
//-----------------------------------------------------------------------------//
/* Error processing when accessing the slave:
 *  err_code  - Error code (all codes defined in the library)
 *  _id       - Slave address
 */
void error_processing(uint8_t err_code, uint8_t _id)
{
  uint8_t current_slave = 0;
  if (err_code)
  {
    for (uint8_t arrAddr = MIN_SLAVE_DEVICES; arrAddr < foundSlaves + 1 ; arrAddr++)
    {
      if (_id == slave_sys_info[arrAddr].id)
      {
        current_slave = arrAddr;
        switch (err_code)
        {
          case 0x01: slave_sys_info[current_slave].err_IllegalFunction ++;
            break;
          case 0x02: slave_sys_info[current_slave].err_IllegalDataAddress ++;
            break;
          case 0x03: slave_sys_info[current_slave].err_IllegalDataValue ++;
            break;
          case 0x04: slave_sys_info[current_slave].err_SlaveDeviceFailure ++;
            break;
          case 0xE0: slave_sys_info[current_slave].err_InvalidSlaveID ++;
            break;
          case 0xE1: slave_sys_info[current_slave].err_InvalidFunction ++;
            break;
          case 0xE2: slave_sys_info[current_slave].err_ResponseTimedOut ++;
            break;
          case 0xE3: slave_sys_info[current_slave].err_InvalidCRC ++;
            break;
        }
      }
    }
    if (!debStruct(_id, foundSlaves))
    {
      Serial.print("ID: ");  Serial.print(_id);  Serial.println(" not found in strucrure!");
    }    
  }
}
//-----------------------------------------------------------------------------//
/* Output in the terminal structure for each slave:
 *  _id           - Slave address
 *  found_slaves  - Number of slaves found  
 */
bool debStruct(uint8_t _id, uint8_t found_slaves)
{
  for (uint8_t addrStruct = MIN_SLAVE_DEVICES; addrStruct <= found_slaves; addrStruct++)
  {
    if (_id == slave_sys_info[addrStruct].id)
    {
      Serial.println("------------------ System Info --------------------");
      Serial.print("id:                  "); Serial.println(slave_sys_info[addrStruct].id);
      Serial.print("serial_number:       "); Serial.println(slave_sys_info[addrStruct].serial_number);
      Serial.print("debug_reg: "); Serial.println(slave_sys_info[addrStruct].debug_reg);
      Serial.println("------------------- Error Info --------------------");
      Serial.print("IllegalFunction:     "); Serial.println(slave_sys_info[addrStruct].err_IllegalFunction);
      Serial.print("IllegalDataAddress:  "); Serial.println(slave_sys_info[addrStruct].err_IllegalDataAddress);
      Serial.print("IllegalDataValue:    "); Serial.println(slave_sys_info[addrStruct].err_IllegalDataValue);
      Serial.print("SlaveDeviceFailure:  "); Serial.println(slave_sys_info[addrStruct].err_SlaveDeviceFailure);
      Serial.print("InvalidSlaveID:      "); Serial.println(slave_sys_info[addrStruct].err_InvalidSlaveID);
      Serial.print("InvalidFunction:     "); Serial.println(slave_sys_info[addrStruct].err_InvalidFunction);
      Serial.print("ResponseTimedOut:    "); Serial.println(slave_sys_info[addrStruct].err_ResponseTimedOut);
      Serial.print("InvalidCRC:          "); Serial.println(slave_sys_info[addrStruct].err_InvalidCRC);
      Serial.println("---------------------------------------------------");

      return true;
    } else if (addrStruct == found_slaves)
      return false;
  }
}
// -------Examles User functions: ---------------------------------------------//
//=============================================================================//
