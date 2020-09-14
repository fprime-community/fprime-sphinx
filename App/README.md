# Sphinx DriverDemo Application

The purpose of this application is to demonstrate a completely assembled application for use on the Sphinx.  
This allows the user to get up and running quickly using many of the Sphinx available peripherals.

## Assumptions

This document makes the following assumptions:

 - User has experience compiling F´ Reference deployment
 - Installed Workbench 3.1 and VxWorks 6.7
 - Installed 6.7 Service Pack 1, DVD-R138711.1-6-01.zip
 - Installed the GNU GCC LEON toolchain
 - Installed the Cobham Gaisler LEON distribution over the VxWorks Platform
 - Built the SPARC/LEON VxWorks kernel and libraries
 - Has a VxWorks kernel configured for the Sphinx
 - Has Sphinx BSP installed

NOTE: the GNU GCC LEON toolchain (referred to as `gr712-vxworks6` in this deployment) is the default toolchain when compiling.

For more information on compiling an F´ deployment, please refer to the README.md provided under F´'s Ref deployment.

## Differences between Sphinx's App deployment and F´ Ref deployment

The Sphinx's App deployment inherits from F´'s reference deployment with the following updates:

 - Removal of SendBuffApp, RecvBuffApp, SignalGen, SocketIpDriver, BlockDriver, and linuxTime component instantiation in topology
 - Changed RateGroup3 to 1 Hz
 - Added RateGroup4 running at .1 Hz
 - Addition of the following components

| Component | Description |
| --------- | ----------- |
| `App::DriverTestComponentImpl::driverTest` | A component to demonstrate interaction with the UART, SPI, and GPIOs on the Sphinx. |
| `App::FSWInfoComponentImpl::fSWInfo` | Reports FSW information like FSW/firmware version. |
| `App::FatalHandlerComponentImpl::fatalHandler` | A component to handle reported fatal events. |
| `App::IdleTaskComponentImpl::idleTask` | Memory scrubber. |
| `Drv::ADCComponentImpl::adc` | Component used to read telemetry from on-chip ADC. |
| `Drv::UartDriverComponentImpl::uartDriver` | Component used to interface with ground over a file descriptor based UART connection. |
| `Drv::SphinxUartOnboardDriverComponentImpl::hw_uart` | Uart driver for on-chip UART. |
| `Drv::SphinxUartOnboardDriverComponentImpl::fw_uart` | Uart driver for firmware UART. |
| `Drv::FPGADriverComponentImpl::fpgaDriver` | Component to drive the RateGroupDrivers. |
| `Drv::SphinxTimeComponentImpl::sphinxTime` | Component to update spacecraft time. Others components can call this component to get time. |
| `Drv::SPIDriverGenericComponentImpl::spiDriverGeneric` | SPI driver component for on-chip SPI. |
| `Drv::FPGASPIDriverComponentImpl::fpgaSpiDriver` | SPI driver component for FPGA SPI. |
| `Drv::GPIODriverComponentImpl::gpioDriver` | GPIO driver component for on-chip GPIOs. |
| `Drv::FpgaGpioDriverComponentImpl::fpgaGpioDriver` | GPIO driver component for FPGA GPIOs. |

The Sphinx's App deployment is available under the `App` directory in `master` branch of the fprime-sphinx repository.

## Topology Interconnect Block Diagrams

For a visual representation of component interconnections in Sphinx App deployment, please take a look at [`DriverDemo-App-Connections.pdf`](./docs/DriverDemo-App-Connections.pdf).

## Configuration

### UART

There are three UART components connected in this topology:

| Component Type | Component's Name | Description |
| -------------- | ------------------------- | ----------- |
| `Drv::UartDriver` | uartDriver | Communication device between FSW and GSE. |
| `Drv::SphinxUartOnboardDriver` | hw_uart | On-chip uart. |
| `Drv::SphinxUartOnboardDriver` | fw_uart | Firmware uart. |

uartDriver is configured to use device `/tyCo/0` at a baudrate of 115200.  
To change the configuration, the user can make the updates on the uartDriver's constructor call.

hw_uart is configured to use uart instance 5, at a baudrate of 115200, and with a processor clock speed of 40 MHz.  
To change the configuration, the user can make the updates to the `hw_uart.init_comp(...)` function call.

fw_uart is configured to use uart instance 7, at a baudrate of 115200, and with a processor clock speed of 40 MHz.  
To change the configuration, the user can make the updates to the `fw_uart.init_comp(...)` function call.


### GPIO

There are two GPIO components connected in this topology:

| Component Type | Component's Name | Description |
| -------------- | ------------------------- | ----------- |
| `Drv::GPIODriver` | gpioDriver | GPIO driver for the on-chip GPIOs. |
| `Drv::FpgaGpioDriver` | fpgaGpioDriver | GPIO driver for the FPGA GPIOs. |


gpioDriver configures two GPIO pins as output pins: 18 and 13.  
This configuration is passed in from the constructor at initialization.  
Use [`Top/GPIODefATB.cpp`](Top/GPIODefATB.cpp) to change configurations.  
For configuration, please note the following:

- Direction (0 = input, 1 = output)
- Interrupt mask (0 = masked, 1 = enabled) (NOTE: interrupts can only be configured as inputs)
- Interrupt polarity (0 = low/falling, 1 = high/rising)
- Interrupt edge (0 = level, 1 = edge) 

fpgaGpioDriver configures nine GPIO pins as input and output pins: 0, 1, 2, 3, 8, 9, 10, 11, 12
This configuration is passed in from the constructor at initialization.  
Use [`Top/FPGAGPIODefATB.cpp`](Top/FPGAGPIODefATB.cpp) to add or remove FPGA GPIO pins.


### SPI

There are two SPI components connected in this topology:

| Component Type | Component's Name | Description |
| -------------- | ------------------------- | ----------- |
| `Drv::SPIDriverGeneric` | spiDriverGeneric | Generic SPI driver component for the on-chip SPI. |
| `Drv::FPGASPIDriver` | fpgaSpiDriver | SPI driver component for the FPGA SPI. |

spiDriverGeneirc is configured to use a SPI width of 8 bits.  
This component takes configuration info passed in from the constructor at initialization.  
Default configuration is used when using the default constructor.  
Please note, the slave select pin used must be enabled in the GPIO configuration.

fpgaSpiDriver is configured to use a SPI width of 16 bits, and use FPGA SPI device 1.  
This configuration is passed in from the constructor at initialization.  

## Capabilities

In addition to the capabilities already provided by the Sphinx App deployment:

 - Users can write and read a byte on the UART, and SPI interfaces.  
