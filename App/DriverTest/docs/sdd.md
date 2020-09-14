App::DriverTest Component
===

## 1. Introduction

`App::DriverTest` is an active F' component that interfaces with UART, SPI, and GPIOs.  
This component allows for single byte reads and writes on the UART and SPI, and allows
toggling GPIOs pins.

## 2. Requirements

The requirements for `App::DriverTest` are as follows:

| Requirement           | Description                                                                                         | Verification Method |
| -----------           | -----------                                                                                         | -------------------
| FPrime-DriverTest-001 | The `App::DriverTest` component shall provide a command to read a byte from a firmware uart.        | Unit test           |
| FPrime-DriverTest-002 | The `App::DriverTest` component shall provide a command to write a byte from a firmware uart.       | Unit test           |
| FPrime-DriverTest-003 | The `App::DriverTest` component shall provide a command to read a byte from an on-chip uart.        | Unit test           |
| FPrime-DriverTest-004 | The `App::DriverTest` component shall provide a command to write a byte from an on-chip uart.       | Unit test           |
| FPrime-DriverTest-005 | The `App::DriverTest` component shall provide a command to read a byte from a firmware spi.         | Unit test           |
| FPrime-DriverTest-006 | The `App::DriverTest` component shall provide a command to write a byte from a firmware spi.        | Unit test           |
| FPrime-DriverTest-007 | The `App::DriverTest` component shall provide a command to read a byte from on on-chip spi.         | Unit test           |
| FPrime-DriverTest-008 | The `App::DriverTest` component shall provide a command to write a byte from on on-chip spi.        | Unit test           |
| FPrime-DriverTest-009 | The `App::DriverTest` component shall provide a command to set an on-chip GPIO pin                  | Unit test           |
| FPrime-DriverTest-010 | The `App::DriverTest` component shall provide a command to clear an on-chip GPIO pin                | Unit test           |
| FPrime-DriverTest-011 | The `App::DriverTest` component shall provide a command to set a firmware GPIO pin                  | Unit test           |
| FPrime-DriverTest-012 | The `App::DriverTest` component shall provide a command to clear a firmware GPIO pin                | Unit test           |

## 3. Design

#### 3.1 Ports

#### 3.1.1 Role Ports

| Port Data Type    | Name           | Direction | Role            |
|-------------------|----------------|-----------|-----------------|
| `Fw::Cmd`         | cmdIn          | input     | Cmd             |
| `Fw::CmdReg`      | cmdRegOut      | output    | CmdRegistration |
| `Fw::CmdResponse` | cmdResponseOut | output    | CmdResponse     |
| `Fw::Log`         | eventOut       | output    | LogEvent        |
| `Svc::TlmChan`    | tlmOut         | output    | Telemetry       |
| `Fw::Time    `    | timeCaller     | output    | TimeGet         |

#### 3.1.2 Component-Specific Ports

| Port Data Type             | Name               | Direction | Kind          | Usage                                      |
|----------------------------|--------------------|-----------|---------------|--------------------------------------------|
| `Drv::SphinxFPGASPI_RW     | fpgaSpiRw          | output    | output        | Send out a read/write spi call.            |
| `Drv::SphinxSPI_Claim      | spiClaim           | output    | output        | Claim the on-chip spi device driver.       |
| `Drv::SphinxSPI_Unclaim    | spiUnclaim         | output    | output        | Unclaim the on-chip spi device driver.     |
| `Drv::SphinxSPI_RW_Generic | spiRw8             | output    | output        | Send out a read/write spi call.            |
| `Drv::SphinxGPIO_WritePin` | gpioSet            | output    | output        | Set an on-chip GPIO pin                    |
| `Drv::SphinxGPIO_WritePin` | gpioClear          | output    | output        | Clear an on-chip GPIO pin                  |
| `Drv::SphinxGPIO_ReadPin`  | gpioRead           | output    | output        | Read the state of an on-chip GPIO pin      |
| `Drv::SphinxGPIO_WritePin` | fpgaGpioSet        | output    | output        | Set an FPGA GPIO pin                       |
| `Drv::SphinxGPIO_WritePin` | fpgaGpioClear      | output    | output        | Clear an FPGA GPIO pin                     |
| `Drv::SphinxGPIO_ReadPin`  | fpgaGpioRead       | output    | output        | Read the state of an FPGA GPIO pin         |
| `Drv::SphinxUart_readPort  | hwUartRx           | output    | output        | Read bytes from the uart                   |
| `Drv::SphinxUart_writePort | hwUartTx           | output    | output        | Write bytes from the uart                  |
| `Drv::SphinxUart_readPort  | fwUartRx           | output    | output        | Read bytes from the uart                   |
| `Drv::SphinxUart_writePort | fwUartTx           | output    | output        | Write bytes from the uart                  |
