# Modbus RTU Communication Library
![Modbus Protocol](https://img.shields.io/badge/Protocol-Modbus%20RTU-blue)
![License](https://img.shields.io/badge/License-MIT-green)

This repository provides an implementation of a Modbus RTU master and slave in C. It allows a microcontroller to communicate over UART using the Modbus protocol, supporting reading and writing of holding registers, CRC validation, and custom string operations.

---

## Table of Contents

1. [Features](#features)
2. [Getting Started](#getting-started)
3. [CRC Calculation](#crc-calculation)
4. [Master Implementation](#master-implementation)

   * [Initialization](#initialization)
   * [Master Transaction](#master-transaction)
   * [Read Holding Registers](#read-holding-registers)
   * [Write Single Register](#write-single-register)
   * [Write Multiple Registers](#write-multiple-registers)
   * [Send String](#send-string)
5. [Slave Implementation](#slave-implementation)

   * [Initialization](#slave-initialization)
   * [Process Request](#process-request)
6. [Usage Example](#usage-example)
7. [Error Handling](#error-handling)

---

## Features

* CRC-16 (Modbus) calculation and validation
* Master support for:

  * Reading holding registers (Function Code 0x03)
  * Writing single registers (Function Code 0x06)
  * Writing multiple registers (Function Code 0x10)
  * Sending custom strings via multiple registers
* Slave support for:

  * Reading holding registers
  * Writing custom string data
  * Automatic CRC checking and error responses
* Simple UART abstraction for portability to different platforms

---

## Getting Started

### Prerequisites

* C compiler (GCC, ARMCC, etc.)
* UART driver or HAL for your platform
* A microcontroller or development board with UART support

### Files

* `modbus_rtu.c` / `modbus_rtu.h`: Core Modbus RTU implementation
* `main.c`: Example usage (not included in this snippet)

Include the header in your project:

```c
#include "modbus_rtu.h"
```

---

## CRC Calculation

The `crc16` function computes the CRC-16 (Modbus) checksum over a byte buffer:

```c
static uint16_t crc16(uint8_t *buffer, uint16_t length);
```

1. Initialize CRC to `0xFFFF`.
2. For each byte, XOR into the CRC.
3. Iterate 8 bits, shifting and applying the polynomial `0xA001` when needed.
4. Returns the 16-bit CRC value.

Used by both master and slave to append and validate the CRC bytes.

---

## Master Implementation

### Initialization

```c
void ModbusMaster_Init(ModbusMaster *ctx,
                       uint8_t slave_address,
                       void (*uart_send)(uint8_t*, uint16_t),
                       uint16_t (*uart_receive)(uint8_t*, uint16_t),
                       void (*delay_ms)(uint32_t));
```

* `ctx`: Pointer to a `ModbusMaster` context struct
* `slave_address`: Default slave ID
* `uart_send`: Function pointer for sending bytes
* `uart_receive`: Function pointer for receiving bytes
* `delay_ms`: Millisecond delay function
* Sets a default timeout of 1000 ms

### Master Transaction

```c
static ModbusError master_transaction(ModbusMaster *ctx,
                                      uint8_t *request,
                                      uint16_t req_len,
                                      uint8_t *response,
                                      uint16_t *resp_len);
```

Core routine to:

1. Append CRC to the `request` buffer
2. Send the request via UART
3. Wait for and receive the response (timeout handling)
4. Validate response length and CRC
5. Check slave address and exception flags
6. Return a `ModbusError` code or `MODBUS_SUCCESS`

### Read Holding Registers

```c
ModbusError ModbusMaster_ReadHoldingRegisters(ModbusMaster *ctx,
                                              uint16_t start_addr,
                                              uint16_t quantity,
                                              uint16_t *values);
```

* Builds function code `0x03` request
* Calls `master_transaction`
* Parses the returned data into the `values` array

### Write Single Register

```c
ModbusError ModbusMaster_WriteSingleRegister(ModbusMaster *ctx,
                                             uint16_t addr,
                                             uint16_t value);
```

* Builds function code `0x06` request
* Uses `master_transaction` to send and validate response

### Write Multiple Registers

```c
ModbusError ModbusMaster_WriteMultipleRegisters(ModbusMaster *ctx,
                                                uint16_t start_addr,
                                                uint16_t quantity,
                                                const uint16_t *values);
```

* Builds function code `0x10` request, including byte count and data
* Validates that `quantity` is within range
* Verifies the slave response matches start address and quantity

### Send String

```c
ModbusError ModbusMaster_SendString(ModbusMaster *ctx,
                                    uint16_t start_addr,
                                    const char *str);
```

* Packs a C string into 16-bit register values
* Sends as multiple registers (max length 50 bytes)
* Facilitates sending ASCII data over Modbus

---

## Slave Implementation

### Slave Initialization

```c
void ModbusSlave_Init(ModbusSlave *slave, uint8_t address);
```

* Zeroes out the `ModbusSlave` struct
* Sets the slave `address`

### Process Request

```c
void ModbusSlave_ProcessRequest(ModbusSlave *slave);
```

Loop:

1. Receive a full frame over UART (implementation-specific)
2. Check minimum length and CRC
3. Verify matching slave address
4. Switch on function code:

   * **Read Holding Registers (0x03)**: Validate address/quantity, build data response
   * **Custom String (0x2B or user-defined)**: Store incoming data into holding registers
   * **Default**: Return exception (illegal function)
5. Append CRC and send the response

---

## Usage Example

```c
#include "modbus_rtu.h"

// UART send/receive implementations for your platform
extern void UART_Send(uint8_t *buf, uint16_t len);
extern uint16_t UART_Receive(uint8_t *buf, uint16_t max_len);
extern void DelayMs(uint32_t ms);

int main(void) {
    ModbusMaster master;
    ModbusMaster_Init(&master, 1, UART_Send, UART_Receive, DelayMs);

    uint16_t regs[10];
    if (ModbusMaster_ReadHoldingRegisters(&master, 0x0000, 10, regs) == MODBUS_SUCCESS) {
        // Process regs[]
    }

    ModbusMaster_WriteSingleRegister(&master, 0x000A, 0x1234);

    return 0;
}
```

---

## Error Handling

The following `ModbusError` codes are returned by master functions:

* `MODBUS_SUCCESS` (0)
* `MODBUS_ERR_TIMEOUT`
* `MODBUS_ERR_INVALID_DATA`
* `MODBUS_ERR_CRC_MISMATCH`
* `MODBUS_ERR_INVALID_ADDRESS`
* `MODBUS_ERR_SLAVE_FAILURE`
* `MODBUS_ERR_INVALID_DATA`

Handle these errors appropriately in your application.

