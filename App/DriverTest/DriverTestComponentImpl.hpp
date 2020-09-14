// ======================================================================
// \title  DriverTestComponentImpl.hpp
// \author ortega
// \brief  hpp file for DriverTest component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef DriverTest_HPP
#define DriverTest_HPP

#include "App/DriverTest/DriverTestComponentAc.hpp"

namespace App {

  class DriverTestComponentImpl :
    public DriverTestComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Constants
      // ----------------------------------------------------------------------

      static const U32 HW_UART_RX_TIME_OUT = 0;
      static const U32 HW_UART_TX_TIME_OUT = 40000;
      static const U32 FW_UART_RX_TIME_OUT = HW_UART_RX_TIME_OUT;
      static const U32 FW_UART_TX_TIME_OUT = HW_UART_TX_TIME_OUT;

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object DriverTest
      //!
      DriverTestComponentImpl(
#if FW_OBJECT_NAMES == 1
          const char *const compName /*!< The component name*/
#else
          void
#endif
      );

      //! Initialize object DriverTest
      //!
      void init(
          const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );

      //! Destroy object DriverTest
      //!
      ~DriverTestComponentImpl(void);

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for PingRecv
      //!
      void PingRecv_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 key /*!< Value to return to pinger*/
      );

      //! Handler implementation for schedIn
      //!
      void schedIn_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          NATIVE_UINT_TYPE context /*!< The call order*/
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      //! Implementation for DRIVER_TEST_SPI_READ_WRITE command handler
      //! Generic Read/Write a byte from/to the spi slave device. Note: before reading a byte from a SPI device, you must write a byte to it.
      void DRIVER_TEST_SPI_READ_WRITE_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U8 slave_select, /*!< SPI slave select gpio pin*/
          U8 byte, /*!< byte to write*/
          U16 timeout /*!< SPI timeout*/
      );

      //! Implementation for DRIVER_TEST_FPGA_SPI_READ_WRITE command handler
      //! Read/Write a byte from/to the FPGA spi slave device
      void DRIVER_TEST_FPGA_SPI_READ_WRITE_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          FpgaSpiSlaveSelect slave_select, /*!< SPI slave device*/
          U8 byte, /*!< byte to write*/
          FpgaSpiBitrate clock_bitrate, /*!< byte to write*/
          U16 timeout, /*!< SPI timeout*/
          FpgaSpiBitOrder bit_order /*!< byte to write*/
      );

      //! Implementation for DRIVER_TEST_GPIO_SET command handler
      //! Set GPIO pin High or Low
      void DRIVER_TEST_GPIO_SET_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U32 gpio_pin, /*!< GPIO pin number*/
          GpioState state /*!< The state to set gpio_pin*/
      );

      //! Implementation for DRIVER_TEST_FPGA_GPIO_SET command handler
      //! Set FPGA GPIO pin High or Low
      void DRIVER_TEST_FPGA_GPIO_SET_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U32 fpga_gpio_pin, /*!< FPGA GPIO pin number*/
          FpgaGpioState state /*!< The state to set fpga_gpio_pin*/
      );

      //! Implementation for DRIVER_TEST_HW_UART_READ command handler
      //! Read a byte from the hardware UART
      void DRIVER_TEST_HW_UART_READ_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Implementation for DRIVER_TEST_HW_UART_WRITE command handler
      //! Write a byte to the hardware UART
      void DRIVER_TEST_HW_UART_WRITE_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U8 byte /*!< Byte to write to HW UART*/
      );

      //! Implementation for DRIVER_TEST_FW_UART_READ command handler
      //! Read a byte from the firmware UART
      void DRIVER_TEST_FW_UART_READ_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Implementation for DRIVER_TEST_FW_UART_WRITE command handler
      //! Write a byte to the firmware UART
      void DRIVER_TEST_FW_UART_WRITE_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U8 byte /*!< Byte to write to FW UART*/
      );

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      I32 release_and_unclaim(U8 ss);

    };

} // end namespace App

#endif
