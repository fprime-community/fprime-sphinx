// ====================================================================== 
// \title  DriverTest.hpp
// \author ortega
// \brief  cpp file for DriverTest test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// 
// ====================================================================== 

#include "Tester.hpp"
#include <fprime-sphinx-drivers/SphinxUartOnboardDriver/SphinxUartOnboardDriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/FPGASPIDriver/FPGASPIDriverComponentImpl.hpp>
#include <Fw/Test/UnitTest.hpp>

#define INSTANCE 0
#define MAX_HISTORY_SIZE 10
#define QUEUE_DEPTH 10

namespace App {

  // ----------------------------------------------------------------------
  // Construction and destruction 
  // ----------------------------------------------------------------------

  Tester ::
    Tester(void) : 
#if FW_OBJECT_NAMES == 1
      DriverTestGTestBase("Tester", MAX_HISTORY_SIZE),
      component("DriverTest"),
#else
      DriverTestGTestBase(MAX_HISTORY_SIZE),
      component(),
#endif
    m_hw_uart_pass(true),
    m_fw_uart_pass(true),
    m_hw_spi_claim(0),
    m_fw_spi_claim(0),
    m_hw_spiRw_pass(true),
    m_fw_spiRw_pass(true),
    m_pingKey(0)
  {
    this->initComponents();
    this->connectPorts();
  }

  Tester ::
    ~Tester(void) 
  {
    
  }

  // ----------------------------------------------------------------------
  // Tests 
  // ----------------------------------------------------------------------
  void Tester ::
    fpgaGpioSet(void)
  {
    TEST_CASE(1000.1.1,"fpga gpio set and clear");
    REQUIREMENT("FPrime-DriverTest-011");
    REQUIREMENT("FPrime-DriverTest-012");

    U32 gpio_pin = 7;
    DriverTestComponentImpl::FpgaGpioState state = DriverTestComponentImpl::FPGA_HIGH;
    this->sendCmd_DRIVER_TEST_FPGA_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestFpgaGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_GPIO_SET, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    state = DriverTestComponentImpl::FPGA_LOW;
    this->sendCmd_DRIVER_TEST_FPGA_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaGpioSetLow(0, gpio_pin);
    ASSERT_EVENTS_DriverTestFpgaGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_GPIO_SET, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fpgaSpiReadWrite(void)
  {
    TEST_CASE(1000.2.1,"fpga spi read and write");
    REQUIREMENT("FPrime-DriverTest-005");
    REQUIREMENT("FPrime-DriverTest-006");

    DriverTestComponentImpl::FpgaSpiSlaveSelect ss = DriverTestComponentImpl::FPGA_SPI_SLAVE_SELECT_4;
    U8 byte = 0xa2;
    DriverTestComponentImpl::FpgaSpiBitrate rate = DriverTestComponentImpl::BITRATE_10_MBS;
    U16 timeout = 1;
    DriverTestComponentImpl::FpgaSpiBitOrder order = DriverTestComponentImpl::MSB_FIRST;

    this->sendCmd_DRIVER_TEST_FPGA_SPI_READ_WRITE(0, 0, ss, byte, rate, timeout, order);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaSpiRw(0, byte, byte, static_cast<U8>(ss));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_SPI_READ_WRITE, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fwUartRead(void)
  {
    TEST_CASE(1000.3.1,"firmware uart read");
    REQUIREMENT("FPrime-DriverTest-001");

    this->sendCmd_DRIVER_TEST_FW_UART_READ(0, 0);
    this->component.doDispatch();
    ASSERT_from_fwUartRx_SIZE(1);
    ASSERT_EVENTS_DriverTestFwUartRead(0, UART_BYTE_VAL);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FW_UART_READ, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fwUartWrite(void)
  {
    TEST_CASE(1000.3.2,"firmware uart write");
    REQUIREMENT("FPrime-DriverTest-002");

    U8 byte = 0xbe;
    this->sendCmd_DRIVER_TEST_FW_UART_WRITE(0, 0, byte);
    this->component.doDispatch();
    ASSERT_from_fwUartTx_SIZE(1);
    ASSERT_EVENTS_DriverTestFwUartWrite(0, byte);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FW_UART_WRITE, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    gpioSet(void)
  {
    TEST_CASE(1000.4.1,"on-chip gpio set and clear");
    REQUIREMENT("FPrime-DriverTest-009");
    REQUIREMENT("FPrime-DriverTest-010");

    U32 gpio_pin = 5;
    DriverTestComponentImpl::GpioState state = DriverTestComponentImpl::HIGH;
    this->sendCmd_DRIVER_TEST_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_GPIO_SET, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    state = DriverTestComponentImpl::LOW;
    this->sendCmd_DRIVER_TEST_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestGpioSetLow(0, gpio_pin);
    ASSERT_EVENTS_DriverTestGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_GPIO_SET, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    hwUartRead(void)
  {
    TEST_CASE(1000.5.1,"on-chip uart read");
    REQUIREMENT("FPrime-DriverTest-003");

    this->sendCmd_DRIVER_TEST_HW_UART_READ(0, 0);
    this->component.doDispatch();
    ASSERT_from_hwUartRx_SIZE(1);
    ASSERT_EVENTS_DriverTestHwUartRead(0, UART_BYTE_VAL);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_HW_UART_READ, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    hwUartWrite(void)
  {
    TEST_CASE(1000.5.2,"on-chip uart write");
    REQUIREMENT("FPrime-DriverTest-004");

    U8 byte = 0xbe;
    this->sendCmd_DRIVER_TEST_HW_UART_WRITE(0, 0, byte);
    this->component.doDispatch();
    ASSERT_from_hwUartTx_SIZE(1);
    ASSERT_EVENTS_DriverTestHwUartWrite(0, byte);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_HW_UART_WRITE, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    ping(void) 
  {
    FW_ASSERT(m_pingKey == 0);
    U32 key = 0xA3BAC362;
    this->invoke_to_PingRecv(0, key);
    this->component.doDispatch();
    ASSERT_EQ(m_pingKey, key);
  }

  void Tester ::
    schedIn(void) 
  {
    U32 cycles = 88;
    for(U32 i = 0; i < cycles; i++)
    {
      this->invoke_to_schedIn(0,0);
      this->component.doDispatch();
    }
  }

  void Tester ::
    spiReadWrite(void) 
  {
    TEST_CASE(1000.6.1,"on-chip spi read and write");
    REQUIREMENT("FPrime-DriverTest-007");
    REQUIREMENT("FPrime-DriverTest-008");

    U8 ss = 0;
    U8 byte = 0xa1;
    U16 timeout = 1;

    this->sendCmd_DRIVER_TEST_SPI_READ_WRITE(0, 0, ss, byte, timeout);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestSpiRw(0, byte, byte, ss);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_SPI_READ_WRITE, 0, Fw::COMMAND_OK);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fpgaGpioSetFail(void)
  {
    U32 gpio_pin = 22;
    DriverTestComponentImpl::FpgaGpioState state = DriverTestComponentImpl::FPGA_HIGH;
    m_fpga_gpio_state_mgr.gpioSetFail(gpio_pin);
    this->sendCmd_DRIVER_TEST_FPGA_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestFpgaGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    m_fpga_gpio_state_mgr.gpioSetSuccess(gpio_pin);
    m_fpga_gpio_state_mgr.gpioReadFail(gpio_pin);
    this->sendCmd_DRIVER_TEST_FPGA_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestFpgaGpioState(0, gpio_pin, static_cast<U32>(DriverTestComponentImpl::FPGA_LOW));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    state = DriverTestComponentImpl::FPGA_LOW;
    m_fpga_gpio_state_mgr.gpioSetSuccess(gpio_pin);
    m_fpga_gpio_state_mgr.gpioClearFail(gpio_pin);
    m_fpga_gpio_state_mgr.gpioReadSuccess(gpio_pin);
    this->sendCmd_DRIVER_TEST_FPGA_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaGpioSetLow(0, gpio_pin);
    ASSERT_EVENTS_DriverTestFpgaGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fpgaSpiReadWriteFail(void)
  {
    DriverTestComponentImpl::FpgaSpiSlaveSelect ss = DriverTestComponentImpl::FPGA_SPI_SLAVE_SELECT_4;
    U8 byte = 0xa2;
    DriverTestComponentImpl::FpgaSpiBitrate rate = DriverTestComponentImpl::BITRATE_10_MBS;
    U16 timeout = 1;
    DriverTestComponentImpl::FpgaSpiBitOrder order = DriverTestComponentImpl::MSB_FIRST;

    m_fw_spiRw_pass = false;
    this->sendCmd_DRIVER_TEST_FPGA_SPI_READ_WRITE(0, 0, ss, byte, rate, timeout, order);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestFpgaSpiRwFail(0, static_cast<U8>(ss), FPGA_SPI_ER_TIMEOUT);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FPGA_SPI_READ_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fwUartReadFail(void)
  {
    m_fw_uart_pass = false;
    this->sendCmd_DRIVER_TEST_FW_UART_READ(0, 0);
    this->component.doDispatch();
    ASSERT_from_fwUartRx_SIZE(1);
    ASSERT_EVENTS_DriverTestFwUartReadFail_SIZE(1);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FW_UART_READ, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    fwUartWriteFail(void)
  {
    U8 byte = 0xbe;
    m_fw_uart_pass = false;
    this->sendCmd_DRIVER_TEST_FW_UART_WRITE(0, 0, byte);
    this->component.doDispatch();
    ASSERT_from_fwUartTx_SIZE(1);
    ASSERT_EVENTS_DriverTestFwUartWriteFail_SIZE(1);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_FW_UART_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    gpioSetFail(void)
  {
    U32 gpio_pin = 11;
    DriverTestComponentImpl::GpioState state = DriverTestComponentImpl::HIGH;
    m_gpio_state_mgr.gpioSetFail(gpio_pin);
    this->sendCmd_DRIVER_TEST_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    m_gpio_state_mgr.gpioSetSuccess(gpio_pin);
    m_gpio_state_mgr.gpioReadFail(gpio_pin);
    this->sendCmd_DRIVER_TEST_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestGpioSetHigh(0, gpio_pin);
    ASSERT_EVENTS_DriverTestGpioState(0, gpio_pin, static_cast<U32>(DriverTestComponentImpl::LOW));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);

    this->clearHistory();
    state = DriverTestComponentImpl::LOW;
    m_gpio_state_mgr.gpioSetSuccess(gpio_pin);
    m_gpio_state_mgr.gpioClearFail(gpio_pin);
    m_gpio_state_mgr.gpioReadSuccess(gpio_pin);
    this->sendCmd_DRIVER_TEST_GPIO_SET(0, 0, gpio_pin, state);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestGpioSetLow(0, gpio_pin);
    ASSERT_EVENTS_DriverTestGpioState(0, gpio_pin, static_cast<U32>(state));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_GPIO_SET, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(2);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    hwUartReadFail(void)
  {
    m_hw_uart_pass = false;
    this->sendCmd_DRIVER_TEST_HW_UART_READ(0, 0);
    this->component.doDispatch();
    ASSERT_from_hwUartRx_SIZE(1);
    ASSERT_EVENTS_DriverTestHwUartReadFail_SIZE(1);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_HW_UART_READ, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    hwUartWriteFail(void)
  {
    U8 byte = 0xbe;
    m_hw_uart_pass = false;
    this->sendCmd_DRIVER_TEST_HW_UART_WRITE(0, 0, byte);
    this->component.doDispatch();
    ASSERT_from_hwUartTx_SIZE(1);
    ASSERT_EVENTS_DriverTestHwUartWriteFail_SIZE(1);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_HW_UART_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
  }

  void Tester ::
    spiReadWriteFail(void)
  {
    U8 ss = 0;
    U8 byte = 0xa1;
    U16 timeout = 1;

    m_fpga_gpio_state_mgr.gpioSetFail(ss);
    this->sendCmd_DRIVER_TEST_SPI_READ_WRITE(0, 0, ss, byte, timeout);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestSpiRwDeselect(0, ss, m_fpga_gpio_state_mgr.getGpioSet(ss));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_SPI_READ_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);

    m_fpga_gpio_state_mgr.gpioSetSuccess(ss);
    clearHistory();
    m_fpga_gpio_state_mgr.gpioClearFail(ss);
    this->sendCmd_DRIVER_TEST_SPI_READ_WRITE(0, 0, ss, byte, timeout);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestSpiRwSelect(0, ss, m_fpga_gpio_state_mgr.getGpioClear(ss));
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_SPI_READ_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);

    m_fpga_gpio_state_mgr.gpioClearSuccess(ss);
    clearHistory();
    m_hw_spiRw_pass = false;
    this->sendCmd_DRIVER_TEST_SPI_READ_WRITE(0, 0, ss, byte, timeout);
    this->component.doDispatch();
    ASSERT_EVENTS_DriverTestSpiRwFail(0, ss, 1);
    ASSERT_CMD_RESPONSE(0, DriverTestComponentImpl::OPCODE_DRIVER_TEST_SPI_READ_WRITE, 0, Fw::COMMAND_EXECUTION_ERROR);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);

    m_hw_spiRw_pass = true;
    clearHistory();
    m_fpga_gpio_state_mgr.gpioSetFail(ss);
    I32 release_status = this->component.release_and_unclaim(ss);
    ASSERT_NE(release_status, 0);
  }

  // ----------------------------------------------------------------------
  // Handlers for typed from ports
  // ----------------------------------------------------------------------

  void Tester ::
    from_PingResponse_handler(
        const NATIVE_INT_TYPE portNum,
        U32 key
    )
  {
    this->pushFromPortEntry_PingResponse(key);
    m_pingKey = key;
  }

  I32 Tester ::
    from_fpgaSpiRw_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *write_buf_ptr,
        U8 *read_buf_ptr,
        U32 nBytes,
        U8 activate_slave,
        U16 timeout,
        U8 clock_bitrate,
        U8 bit_order,
        U32 delay
    )
  {
    this->pushFromPortEntry_fpgaSpiRw(write_buf_ptr, read_buf_ptr, nBytes, activate_slave, timeout, clock_bitrate, bit_order, delay);
    for(U32 i = 0; i < nBytes; i++) {
      read_buf_ptr[i] = write_buf_ptr[i];
    }
    return (m_fw_spiRw_pass) ? FPGA_SPI_OK : FPGA_SPI_ER_TIMEOUT;
  }

  I32 Tester ::
    from_spiUnclaim_handler(
        const NATIVE_INT_TYPE portNum
    )
  {
    this->pushFromPortEntry_spiUnclaim();
    m_hw_spi_claim = 0;
    return m_hw_spi_claim;
  }

  I32 Tester ::
    from_spiClaim_handler(
        const NATIVE_INT_TYPE portNum
    )
  {
    this->pushFromPortEntry_spiClaim();
    I32 val = m_hw_spi_claim;
    m_hw_spi_claim = 1;
    return val;
  }

  I32 Tester ::
    from_spiRw8_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *write_buf_ptr,
        U8 *read_buf_ptr,
        U32 nWords,
        U32 timeout_uS
    )
  {
    this->pushFromPortEntry_spiRw8(write_buf_ptr, read_buf_ptr, nWords, timeout_uS);
    for(U32 i = 0; i < nWords; i++) {
      read_buf_ptr[i] = write_buf_ptr[i];
    }
    return (m_hw_spiRw_pass) ? 0 : 1;
  }

  I32 Tester ::
    from_gpioSet_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_gpioSet(pin_num);
    m_gpio_state_mgr.setGpioVal(pin_num, 1);
    return m_gpio_state_mgr.getGpioSet(pin_num);
  }

  I32 Tester ::
    from_gpioClear_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_gpioClear(pin_num);
    m_gpio_state_mgr.setGpioVal(pin_num, 0);
    return m_gpio_state_mgr.getGpioClear(pin_num);
  }

  U32 Tester ::
    from_gpioRead_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_gpioRead(pin_num);
    return m_gpio_state_mgr.getGpioVal(pin_num);
  }

  I32 Tester ::
    from_fpgaGpioSet_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_fpgaGpioSet(pin_num);
    m_fpga_gpio_state_mgr.setGpioVal(pin_num, 1);
    return m_fpga_gpio_state_mgr.getGpioSet(pin_num);
  }

  I32 Tester ::
    from_fpgaGpioClear_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_fpgaGpioClear(pin_num);
    m_fpga_gpio_state_mgr.setGpioVal(pin_num, 0);
    return m_fpga_gpio_state_mgr.getGpioClear(pin_num);
  }

  U32 Tester ::
    from_fpgaGpioRead_handler(
        const NATIVE_INT_TYPE portNum,
        U32 pin_num
    )
  {
    this->pushFromPortEntry_fpgaGpioRead(pin_num);
    return m_fpga_gpio_state_mgr.getGpioVal(pin_num);
  }

  I32 Tester ::
    from_hwUartRx_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *read_buf_ptr,
        U32 nBytes,
        U32 timeout
    )
  {
    this->pushFromPortEntry_hwUartRx(read_buf_ptr, nBytes, timeout);
    read_buf_ptr[0] = UART_BYTE_VAL;
    return (m_hw_uart_pass) ?
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK:
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_PARITY;
  }

  I32 Tester ::
    from_hwUartTx_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *write_buf_ptr,
        U32 nBytes,
        U32 timeout
    )
  {
    this->pushFromPortEntry_hwUartTx(write_buf_ptr, nBytes, timeout);
    return (m_hw_uart_pass) ?
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK:
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_PARITY;
  }

  I32 Tester ::
    from_fwUartRx_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *read_buf_ptr,
        U32 nBytes,
        U32 timeout
    )
  {
    this->pushFromPortEntry_fwUartRx(read_buf_ptr, nBytes, timeout);
    read_buf_ptr[0] = UART_BYTE_VAL;
    return (m_fw_uart_pass) ?
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK:
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_PARITY;
  }

  I32 Tester ::
    from_fwUartTx_handler(
        const NATIVE_INT_TYPE portNum,
        U8 *write_buf_ptr,
        U32 nBytes,
        U32 timeout
    )
  {
    this->pushFromPortEntry_fwUartTx(write_buf_ptr, nBytes, timeout);
    return (m_fw_uart_pass) ?
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK:
      Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_PARITY;
  }

  // ----------------------------------------------------------------------
  // Helper methods 
  // ----------------------------------------------------------------------

  void Tester ::
    connectPorts(void) 
  {

    // PingRecv
    this->connect_to_PingRecv(
        0,
        this->component.get_PingRecv_InputPort(0)
    );

    // cmdIn
    this->connect_to_cmdIn(
        0,
        this->component.get_cmdIn_InputPort(0)
    );

    // schedIn
    this->connect_to_schedIn(
        0,
        this->component.get_schedIn_InputPort(0)
    );

    // cmdRegOut
    this->component.set_cmdRegOut_OutputPort(
        0, 
        this->get_from_cmdRegOut(0)
    );

    // eventOut
    this->component.set_eventOut_OutputPort(
        0, 
        this->get_from_eventOut(0)
    );

    // PingResponse
    this->component.set_PingResponse_OutputPort(
        0, 
        this->get_from_PingResponse(0)
    );

    // cmdResponseOut
    this->component.set_cmdResponseOut_OutputPort(
        0, 
        this->get_from_cmdResponseOut(0)
    );

    // timeCaller
    this->component.set_timeCaller_OutputPort(
        0, 
        this->get_from_timeCaller(0)
    );

    // fpgaSpiRw
    this->component.set_fpgaSpiRw_OutputPort(
        0, 
        this->get_from_fpgaSpiRw(0)
    );

    // spiUnclaim
    this->component.set_spiUnclaim_OutputPort(
        0, 
        this->get_from_spiUnclaim(0)
    );

    // spiClaim
    this->component.set_spiClaim_OutputPort(
        0, 
        this->get_from_spiClaim(0)
    );

    // spiRw8
    this->component.set_spiRw8_OutputPort(
        0, 
        this->get_from_spiRw8(0)
    );

    // gpioSet
    this->component.set_gpioSet_OutputPort(
        0, 
        this->get_from_gpioSet(0)
    );

    // gpioClear
    this->component.set_gpioClear_OutputPort(
        0, 
        this->get_from_gpioClear(0)
    );

    // gpioRead
    this->component.set_gpioRead_OutputPort(
        0, 
        this->get_from_gpioRead(0)
    );

    // fpgaGpioSet
    this->component.set_fpgaGpioSet_OutputPort(
        0, 
        this->get_from_fpgaGpioSet(0)
    );

    // fpgaGpioClear
    this->component.set_fpgaGpioClear_OutputPort(
        0, 
        this->get_from_fpgaGpioClear(0)
    );

    // fpgaGpioRead
    this->component.set_fpgaGpioRead_OutputPort(
        0, 
        this->get_from_fpgaGpioRead(0)
    );

    // hwUartRx
    this->component.set_hwUartRx_OutputPort(
        0, 
        this->get_from_hwUartRx(0)
    );

    // hwUartTx
    this->component.set_hwUartTx_OutputPort(
        0, 
        this->get_from_hwUartTx(0)
    );

    // fwUartRx
    this->component.set_fwUartRx_OutputPort(
        0, 
        this->get_from_fwUartRx(0)
    );

    // fwUartTx
    this->component.set_fwUartTx_OutputPort(
        0, 
        this->get_from_fwUartTx(0)
    );

    // LogText
    this->component.set_LogText_OutputPort(
        0, 
        this->get_from_LogText(0)
    );




  }

  void Tester ::
    initComponents(void) 
  {
    this->init();
    this->component.init(
        QUEUE_DEPTH, INSTANCE
    );
  }

} // end namespace App
