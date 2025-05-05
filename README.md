Modbus RTU Library
Overview
This is a lightweight Modbus RTU (Remote Terminal Unit) library implemented in C, supporting both master and slave functionality. The library provides basic Modbus operations over serial communication with CRC error checking.

Features
Master Functionality
Read Holding Registers (Function Code 0x03)

Write Single Register (Function Code 0x06)

Write Multiple Registers (Function Code 0x10)

Custom string transmission function

CRC-16 error checking

Configurable timeout handling

Slave Functionality
Process incoming Modbus requests

Handle Holding Registers (Function Code 0x03)

Support for custom string function (Function Code 0x41)

Automatic error response generation

CRC verification

Usage
Master Initialization
c
ModbusMaster master;
ModbusMaster_Init(&master, 
                 SLAVE_ADDRESS, 
                 uart_send_function, 
                 uart_receive_function,
                 delay_ms_function);
Common Master Operations
Read Holding Registers:

c
uint16_t values[10];
ModbusError err = ModbusMaster_ReadHoldingRegisters(&master, 0, 10, values);
Write Single Register:

c
ModbusError err = ModbusMaster_WriteSingleRegister(&master, 0, 0x1234);
Write Multiple Registers:

c
uint16_t values[] = {0x1234, 0x5678};
ModbusError err = ModbusMaster_WriteMultipleRegisters(&master, 0, 2, values);
Send String:

c
ModbusError err = ModbusMaster_SendString(&master, 0, "Hello Modbus!");
Slave Implementation
c
ModbusSlave slave;
ModbusSlave_Init(&slave, SLAVE_ADDRESS);

// Main loop
while(1) {
    ModbusSlave_ProcessRequest(&slave);
    // Your holding registers are accessible via slave.holding_registers[]
}
Configuration
Define MODBUS_MAX_FRAME_SIZE - Maximum frame size (default should be at least 256)

Define MODBUS_MAX_HOLDING_REGISTERS - Number of holding registers (default 100)

Implement the required platform-specific functions:

UART send/receive

Delay function (milliseconds)

Error Handling
The library returns ModbusError codes:

MODBUS_SUCCESS - Operation completed successfully

MODBUS_ERR_TIMEOUT - No response from slave

MODBUS_ERR_CRC_MISMATCH - CRC check failed

MODBUS_ERR_INVALID_ADDRESS - Address mismatch

MODBUS_ERR_SLAVE_FAILURE - Slave returned error

MODBUS_ERR_INVALID_DATA - Invalid request/response data

Limitations
Currently supports only a subset of Modbus functions

Requires platform-specific UART and timing implementations

No support for broadcast messages

Limited error recovery mechanisms

Dependencies
Standard C library (for memset, etc.)

Platform-specific UART driver

Millisecond delay function

License
MIT License - Free for commercial and personal use.
