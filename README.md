# Modbus RTU Library

![Modbus Protocol](https://img.shields.io/badge/Protocol-Modbus%20RTU-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A lightweight C implementation of Modbus RTU protocol with both master and slave functionality, designed for embedded systems.

## Table of Contents
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
  - [Master Mode](#master-mode)
  - [Slave Mode](#slave-mode)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Error Handling](#error-handling)
- [Examples](#examples)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [License](#license)

## Features

### Master Functionality
- Read Holding Registers (FC 0x03)
- Write Single Register (FC 0x06)
- Write Multiple Registers (FC 0x10)
- Custom string transmission
- CRC-16 error checking
- Configurable timeout handling

### Slave Functionality
- Automatic request processing
- Holding register access
- Custom function code support
- Error response generation
- CRC verification

## Getting Started

### Prerequisites
- C compiler (GCC, ARMCC, etc.)
- UART peripheral with interrupts
- Basic timing functions (millisecond delay)

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/modbus-rtu.git
