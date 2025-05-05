#include "modbus_rtu.h"

// CRC calculation
static uint16_t crc16(uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (uint8_t i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Master Implementation
void ModbusMaster_Init(ModbusMaster *ctx, uint8_t slave_address, 
                      void (*uart_send)(uint8_t*, uint16_t), 
                      uint16_t (*uart_receive)(uint8_t*, uint16_t),
                      void (*delay_ms)(uint32_t)) {
    ctx->slave_address = slave_address;
    ctx->uart_send = uart_send;
    ctx->uart_receive = uart_receive;
    ctx->delay_ms = delay_ms;
    ctx->timeout_ms = 1000;
}

static ModbusError master_transaction(ModbusMaster *ctx, uint8_t *request, 
                                    uint16_t req_len, uint8_t *response, 
                                    uint16_t *resp_len) {
    uint16_t crc = crc16(request, req_len);
    request[req_len++] = crc & 0xFF;
    request[req_len++] = (crc >> 8) & 0xFF;
    
    ctx->uart_send(request, req_len);
    
    uint32_t start_time = 0; // Implement timer functionality based on your system
    while((*resp_len = ctx->uart_receive(response, MODBUS_MAX_FRAME_SIZE)) == 0) {
        ctx->delay_ms(1);
        if(++start_time > ctx->timeout_ms) return MODBUS_ERR_TIMEOUT;
    }
    
    if(*resp_len < 4) return MODBUS_ERR_INVALID_DATA;
    
    crc = crc16(response, *resp_len - 2);
    if((response[*resp_len-2] != (crc & 0xFF)) || 
       (response[*resp_len-1] != ((crc >> 8) & 0xFF))) {
        return MODBUS_ERR_CRC_MISMATCH;
    }
    
    if(response[0] != ctx->slave_address) return MODBUS_ERR_INVALID_ADDRESS;
    if(response[1] & 0x80) return MODBUS_ERR_SLAVE_FAILURE;
    
    return MODBUS_SUCCESS;
}

ModbusError ModbusMaster_ReadHoldingRegisters(ModbusMaster *ctx, uint16_t start_addr, 
                                            uint16_t quantity, uint16_t *values) {
    uint8_t request[8];
    uint8_t response[MODBUS_MAX_FRAME_SIZE];
    uint16_t resp_len;
    
    request[0] = ctx->slave_address;
    request[1] = MODBUS_FC_READ_HOLDING_REGISTERS;
    request[2] = start_addr >> 8;
    request[3] = start_addr & 0xFF;
    request[4] = quantity >> 8;
    request[5] = quantity & 0xFF;
    
    ModbusError err = master_transaction(ctx, request, 6, response, &resp_len);
    if(err != MODBUS_SUCCESS) return err;
    
    uint8_t byte_count = response[2];
    for(uint16_t i = 0; i < byte_count/2; i++) {
        values[i] = (response[3 + i*2] << 8) | response[4 + i*2];
    }
    
    return MODBUS_SUCCESS;
}

ModbusError ModbusMaster_WriteSingleRegister(ModbusMaster *ctx, uint16_t addr, uint16_t value) {
    uint8_t request[8];
    uint8_t response[MODBUS_MAX_FRAME_SIZE];
    uint16_t resp_len;
    
    request[0] = ctx->slave_address;
    request[1] = MODBUS_FC_WRITE_SINGLE_REGISTER;
    request[2] = addr >> 8;
    request[3] = addr & 0xFF;
    request[4] = value >> 8;
    request[5] = value & 0xFF;
    
    return master_transaction(ctx, request, 6, response, &resp_len);
}

ModbusError ModbusMaster_SendString(ModbusMaster *ctx, uint16_t start_addr, const char *str) {
    uint16_t length = strlen(str);
    if(length > 50) return MODBUS_ERR_INVALID_DATA;
    
    uint16_t registers[15] = {0};
    for(uint8_t i = 0; i < 15; i++) {
        registers[i] = (str[i*2] << 8) | (str[i*2 + 1] & 0xFF);
    }
    
    return ModbusMaster_WriteMultipleRegisters(ctx, start_addr, 15, registers);
}

// Slave Implementation
void ModbusSlave_Init(ModbusSlave *slave, uint8_t address) {
    memset(slave, 0, sizeof(ModbusSlave));
    slave->address = address;
}

void ModbusSlave_ProcessRequest(ModbusSlave *slave) {
    uint8_t frame[MODBUS_MAX_FRAME_SIZE];
    uint16_t frame_length = 0;
    // Receive frame (implement UART receive logic)
    // ...
    
    if(frame_length < 4) return;
    
    // Verify CRC
    uint16_t crc = crc16(frame, frame_length - 2);
    if(frame[frame_length-2] != (crc & 0xFF) || 
       frame[frame_length-1] != (crc >> 8)) return;
    
    if(frame[0] != slave->address) return;
    
    uint8_t response[MODBUS_MAX_FRAME_SIZE];
    uint16_t resp_len = 0;
    response[resp_len++] = slave->address;
    
    switch(frame[1]) {
        case MODBUS_FC_READ_HOLDING_REGISTERS: {
            uint16_t start_addr = (frame[2] << 8) | frame[3];
            uint16_t quantity = (frame[4] << 8) | frame[5];
            
            if(start_addr + quantity > MODBUS_MAX_HOLDING_REGISTERS) {
                response[1] = frame[1] | 0x80;
                response[2] = 0x02; // illegal data address
                resp_len = 3;
                break;
            }
            
            response[1] = MODBUS_FC_READ_HOLDING_REGISTERS;
            response[2] = quantity * 2;
            resp_len = 3;
            
            for(uint16_t i = 0; i < quantity; i++) {
                response[resp_len++] = slave->holding_registers[start_addr + i] >> 8;
                response[resp_len++] = slave->holding_registers[start_addr + i] & 0xFF;
            }
            break;
        }
        
        case MODBUS_FC_CUSTOM_STRING: {
            uint16_t start_addr = (frame[2] << 8) | frame[3];
            uint16_t byte_count = frame[4];
            
            if(byte_count > 30 || start_addr + byte_count/2 > MODBUS_MAX_HOLDING_REGISTERS) {
                response[1] = frame[1] | 0x80;
                response[2] = 0x03; // Illegal data value
                resp_len = 3;
                break;
            }
            
            for(uint16_t i = 0; i < byte_count/2; i++) {
                slave->holding_registers[start_addr + i] = (frame[5 + i*2] << 8) | frame[6 + i*2];
            }
            
            response[1] = MODBUS_FC_CUSTOM_STRING;
            response[2] = start_addr >> 8;
            response[3] = start_addr & 0xFF;
            response[4] = byte_count >> 8;
            response[5] = byte_count & 0xFF;
            resp_len = 6;
            break;
        }
        
        // Add other function code handlers
        
        default:
            response[1] = frame[1] | 0x80;
            response[2] = 0x01; // Illegal function
            resp_len = 3;
            break;
    }
    
    // Calculate and append CRC
    crc = crc16(response, resp_len);
    response[resp_len++] = crc & 0xFF;
    response[resp_len++] = (crc >> 8) & 0xFF;
    
    // Send response
    // Implement UART send logic
}

ModbusError ModbusMaster_WriteMultipleRegisters(ModbusMaster *ctx, uint16_t start_addr, 
                                              uint16_t quantity, const uint16_t *values) {
    if(quantity < 1 || quantity > MODBUS_MAX_HOLDING_REGISTERS) {
        return MODBUS_ERR_INVALID_DATA;
    }

    uint8_t request[6 + 2 * quantity]; // Header + data
    uint8_t response[MODBUS_MAX_FRAME_SIZE];
    uint16_t resp_len;

    request[0] = ctx->slave_address;
    request[1] = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
    request[2] = start_addr >> 8;
    request[3] = start_addr & 0xFF;
    request[4] = quantity >> 8;
    request[5] = quantity & 0xFF;
    request[6] = quantity * 2;  // Byte count

    // Pack register values into request
    for(uint16_t i = 0; i < quantity; i++) {
        request[7 + (i*2)] = values[i] >> 8;
        request[8 + (i*2)] = values[i] & 0xFF;
    }

    ModbusError err = master_transaction(ctx, request, 7 + (quantity * 2), response, &resp_len);
    if(err != MODBUS_SUCCESS) {
        return err;
    }

    // Verify response format (address, function code, start address, quantity)
    if(resp_len != 8 ||  // 6 bytes response + 2 bytes CRC
       response[1] != MODBUS_FC_WRITE_MULTIPLE_REGISTERS ||
       (response[2] << 8 | response[3]) != start_addr ||
       (response[4] << 8 | response[5]) != quantity) {
        return MODBUS_ERR_INVALID_DATA;
    }

    return MODBUS_SUCCESS;
}
																							
