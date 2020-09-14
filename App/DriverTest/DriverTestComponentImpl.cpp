// ======================================================================
// \title  DriverTestComponentImpl.cpp
// \author ortega
// \brief  cpp file for DriverTest component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================


#include <App/DriverTest/DriverTestComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include <fprime-sphinx-drivers/SphinxUartOnboardDriver/SphinxUartOnboardDriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/FPGASPIDriver/FPGASPIDriverComponentImpl.hpp>

namespace App {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  DriverTestComponentImpl ::
#if FW_OBJECT_NAMES == 1
    DriverTestComponentImpl(
        const char *const compName
    ) :
      DriverTestComponentBase(compName)
#else
    DriverTestComponentImpl(void)
#endif
  {

  }

  void DriverTestComponentImpl ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    DriverTestComponentBase::init(queueDepth, instance);
  }

  DriverTestComponentImpl ::
    ~DriverTestComponentImpl(void)
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void DriverTestComponentImpl ::
    PingRecv_handler(
        const NATIVE_INT_TYPE portNum,
        U32 key
    )
  {
    if(this->isConnected_PingResponse_OutputPort(0))
    {
      this->PingResponse_out(0, key);
    }
  }

  void DriverTestComponentImpl ::
    schedIn_handler(
        const NATIVE_INT_TYPE portNum,
        NATIVE_UINT_TYPE context
    )
  {
  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void DriverTestComponentImpl ::
    DRIVER_TEST_SPI_READ_WRITE_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U8 slave_select,
        U8 byte,
        U16 timeout
    )
  {
    U8 readVal;
    I32 claim_status = 0;
    I32 gpio_status = 0;
    I32 rw_status = 0;
    I32 release_status = 0;

    // claim spi driver
    claim_status = this->spiClaim_out(0);
    FW_ASSERT(claim_status == 0, claim_status);

    // slave select
    gpio_status = fpgaGpioSet_out(0, slave_select); // deselect
    if(gpio_status != 0) {
      this->log_WARNING_HI_DriverTestSpiRwDeselect(slave_select, gpio_status);
      (void)this->release_and_unclaim(slave_select);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
      return;
    }

    gpio_status = fpgaGpioClear_out(0, slave_select); // select
    if(gpio_status != 0) {
      this->log_WARNING_HI_DriverTestSpiRwSelect(slave_select, gpio_status);
      (void)this->release_and_unclaim(slave_select);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
      return;
    }

    // read/write
    rw_status = this->spiRw8_out(0, &byte, &readVal, 1, timeout);
    if(rw_status != 0) {
      this->log_WARNING_HI_DriverTestSpiRwFail(slave_select, rw_status);
      (void)this->release_and_unclaim(slave_select);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
      return;
    }

    // release and unclaim
    release_status = this->release_and_unclaim(slave_select);
    if(release_status == 0) {
      this->log_ACTIVITY_HI_DriverTestSpiRw(byte, readVal, slave_select);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestSpiRwReleaseFail(slave_select, release_status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_FPGA_SPI_READ_WRITE_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        FpgaSpiSlaveSelect slave_select,
        U8 byte,
        FpgaSpiBitrate clock_bitrate,
        U16 timeout,
        FpgaSpiBitOrder bit_order
    )
  {
    U8 readVal;
    I32 status = FPGA_SPI_OK;
    U8 ss = static_cast<U8>(slave_select);

    status = this->fpgaSpiRw_out(0, &byte, &readVal, 1, ss, timeout, clock_bitrate, bit_order, 0);
    if(status == FPGA_SPI_OK) {
      this->log_ACTIVITY_HI_DriverTestFpgaSpiRw(byte, readVal, ss);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestFpgaSpiRwFail(ss, status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_GPIO_SET_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 gpio_pin,
        GpioState state
    )
  {
    I32 status = 0;
    U32 curVal = 0;
    U32 expVal = (state == HIGH) ? 1 : 0;

    switch(state)
    {
      case HIGH:
        this->log_ACTIVITY_HI_DriverTestGpioSetHigh(gpio_pin);
        status = this->gpioSet_out(0, gpio_pin);
        break;
      case LOW:
        this->log_ACTIVITY_HI_DriverTestGpioSetLow(gpio_pin);
        status = this->gpioClear_out(0, gpio_pin);
        break;
      default:
        this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
        return;
    }

    curVal = this->gpioRead_out(0, gpio_pin);
    this->log_ACTIVITY_HI_DriverTestGpioState(gpio_pin, curVal);
    if(status != 0 || curVal != expVal) {
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    } else {
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_FPGA_GPIO_SET_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 fpga_gpio_pin,
        FpgaGpioState state
    )
  {
    I32 status = 0;
    U32 curVal = 0;
    U32 expVal = (state == FPGA_HIGH) ? 1 : 0;

    switch(state)
    {
      case FPGA_HIGH:
        this->log_ACTIVITY_HI_DriverTestFpgaGpioSetHigh(fpga_gpio_pin);
        status = this->fpgaGpioSet_out(0, fpga_gpio_pin);
        break;
      case FPGA_LOW:
        this->log_ACTIVITY_HI_DriverTestFpgaGpioSetLow(fpga_gpio_pin);
        status = this->fpgaGpioClear_out(0, fpga_gpio_pin);
        break;
      default:
        this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
        return;
    }

    curVal = this->fpgaGpioRead_out(0, fpga_gpio_pin);
    this->log_ACTIVITY_HI_DriverTestFpgaGpioState(fpga_gpio_pin, curVal);
    if(status != 0 || curVal != expVal) {
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    } else {
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_HW_UART_READ_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    U8 byte = '0';
    I32 status = Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK;
    status = this->hwUartRx_out(0, &byte, 1, HW_UART_RX_TIME_OUT);
    if(status == Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK) {
      this->log_ACTIVITY_HI_DriverTestHwUartRead(byte);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestHwUartReadFail(status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_HW_UART_WRITE_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U8 byte
    )
  {
    I32 status = Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK;
    status = this->hwUartTx_out(0, &byte, 1, HW_UART_TX_TIME_OUT);
    if(status == Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK) {
      this->log_ACTIVITY_HI_DriverTestHwUartWrite(byte);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestHwUartWriteFail(status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_FW_UART_READ_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    U8 byte = '0';
    I32 status = Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK;
    status = this->fwUartRx_out(0, &byte, 1, FW_UART_RX_TIME_OUT);
    if(status == Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK) {
      this->log_ACTIVITY_HI_DriverTestFwUartRead(byte);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestFwUartReadFail(status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void DriverTestComponentImpl ::
    DRIVER_TEST_FW_UART_WRITE_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U8 byte
    )
  {
    I32 status = Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK;
    status = this->fwUartTx_out(0, &byte, 1, FW_UART_TX_TIME_OUT);
    if(status == Drv::SphinxUartOnboardDriverComponentImpl::IOErrorBit_OK) {
      this->log_ACTIVITY_HI_DriverTestFwUartWrite(byte);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
    } else {
      this->log_WARNING_HI_DriverTestFwUartWriteFail(status);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  // ----------------------------------------------------------------------
  // Helper Functions
  // ----------------------------------------------------------------------

  I32 DriverTestComponentImpl ::
    release_and_unclaim(U8 ss)
  {
    I32 gpio_status;
    I32 spi_status;

    //Set slaveSelect gpio pin high (deselect)
    gpio_status = this->fpgaGpioSet_out(0, ss);

    //Unclaim the spi driver
    spi_status = this->spiUnclaim_out(0);
    FW_ASSERT(spi_status == 0);

    return gpio_status;
  }


} // end namespace App
