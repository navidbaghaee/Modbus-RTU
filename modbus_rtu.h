#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include "main.h"

// Modbus addressing limits
#define MODBUS_MAX_COILS                2000
#define MODBUS_MAX_DISCRETE_INPUTS      2000
#define MODBUS_MAX_HOLDING_REGISTERS    125
#define MODBUS_MAX_INPUT_REGISTERS      125
#define MODBUS_MAX_FRAME_SIZE          256

// Modbus function codes
typedef enum {
    MODBUS_FC_READ_COILS = 0x01,
    MODBUS_FC_READ_DISCRETE_INPUTS = 0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS = 0x03,
    MODBUS_FC_READ_INPUT_REGISTERS = 0x04,
    MODBUS_FC_WRITE_SINGLE_COIL = 0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER = 0x06,
    MODBUS_FC_WRITE_MULTIPLE_COILS = 0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS = 0x10,
    MODBUS_FC_CUSTOM_STRING = 0x16  // Custom function code for string transfer
} ModbusFunctionCode;

// Error codes
typedef enum {
    MODBUS_SUCCESS = 0,
    MODBUS_ERR_INVALID_ADDRESS,
    MODBUS_ERR_INVALID_DATA,
    MODBUS_ERR_CRC_MISMATCH,
    MODBUS_ERR_TIMEOUT,
    MODBUS_ERR_SLAVE_FAILURE
} ModbusError;

// Slave device data structure
typedef struct {
    uint8_t address;
    uint8_t coils[MODBUS_MAX_COILS/8 + 1];
    uint8_t discrete_inputs[MODBUS_MAX_DISCRETE_INPUTS/8 + 1];
    uint16_t holding_registers[MODBUS_MAX_HOLDING_REGISTERS];
    uint16_t input_registers[MODBUS_MAX_INPUT_REGISTERS];
} ModbusSlave;

// Master device context
typedef struct {
    uint8_t slave_address;
    uint32_t timeout_ms;
    // Hardware abstraction layer functions
    void (*uart_send)(uint8_t *data, uint16_t length);
    uint16_t (*uart_receive)(uint8_t *buffer, uint16_t max_length);
    void (*delay_ms)(uint32_t ms);
} ModbusMaster;

// Public API for Master
void ModbusMaster_Init(ModbusMaster *ctx, uint8_t slave_address, 
                      void (*uart_send)(uint8_t*, uint16_t), 
                      uint16_t (*uart_receive)(uint8_t*, uint16_t),
                      void (*delay_ms)(uint32_t));

ModbusError ModbusMaster_ReadHoldingRegisters(ModbusMaster *ctx, uint16_t start_addr, 
                                            uint16_t quantity, uint16_t *values);
ModbusError ModbusMaster_WriteSingleRegister(ModbusMaster *ctx, uint16_t addr, uint16_t value);
ModbusError ModbusMaster_WriteMultipleRegisters(ModbusMaster *ctx, uint16_t start_addr, 
                                              uint16_t quantity, const uint16_t *values);
ModbusError ModbusMaster_SendString(ModbusMaster *ctx, uint16_t start_addr, const char *str);

// Public API for Slave
void ModbusSlave_Init(ModbusSlave *slave, uint8_t address);
void ModbusSlave_ProcessRequest(ModbusSlave *slave);

#endif // MODBUS_H








