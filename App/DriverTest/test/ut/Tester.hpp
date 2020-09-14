// ======================================================================
// \title  DriverTest/test/ut/Tester.hpp
// \author ortega
// \brief  hpp file for DriverTest test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "App/DriverTest/DriverTestComponentImpl.hpp"

namespace App {

  class Tester :
    public DriverTestGTestBase
  {

      // ----------------------------------------------------------------------
      // Helper Class
      // ----------------------------------------------------------------------
      class GpioStateMgr
      {
        typedef struct
        {
          I32 set_status;
          I32 clear_status;
          I32 read_status;
          U32 pin_value;
        } gpio_state;

        static const U32 GPIO_ARRAY_SIZE = 60;

        public:
          GpioStateMgr(void) { memset(m_gpios, 0, sizeof(m_gpios[0]) * GPIO_ARRAY_SIZE); }

          I32 getGpioSet(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); return m_gpios[index].set_status; }
          I32 getGpioClear(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); return m_gpios[index].clear_status; }
          I32 getGpioRead(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); return m_gpios[index].read_status; }
          U32 getGpioVal(U32 index) {
            FW_ASSERT(index < GPIO_ARRAY_SIZE, index);
            if(m_gpios[index].read_status == 0) {
              return m_gpios[index].pin_value;
            }
            else {
              // read pin will fail. return opposite value
              return (m_gpios[index].pin_value == 0) ? 1 : 0;
            }
          }
          void gpioSetFail(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].set_status = 1; }
          void gpioClearFail(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].clear_status = 1; }
          void gpioReadFail(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].read_status = 1; }
          void gpioSetSuccess(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].set_status = 0; }
          void gpioClearSuccess(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].clear_status = 0; }
          void gpioReadSuccess(U32 index) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].read_status = 0; }
          void setGpioVal(U32 index, U32 val) { FW_ASSERT(index < GPIO_ARRAY_SIZE, index); m_gpios[index].pin_value = val; }
        private:
          gpio_state m_gpios[GPIO_ARRAY_SIZE];
      };

      // ----------------------------------------------------------------------
      // Construction and destruction
      // ----------------------------------------------------------------------

    public:

      //! Construct object Tester
      //!
      Tester(void);

      //! Destroy object Tester
      //!
      ~Tester(void);

    public:

      // ----------------------------------------------------------------------
      // Tests
      // ----------------------------------------------------------------------

      void fpgaGpioSet(void);
      void fpgaSpiReadWrite(void);
      void fwUartRead(void);
      void fwUartWrite(void);
      void gpioSet(void);
      void hwUartRead(void);
      void hwUartWrite(void);
      void ping(void);
      void schedIn(void);
      void spiReadWrite(void);

      void fpgaGpioSetFail(void);
      void fpgaSpiReadWriteFail(void);
      void fwUartReadFail(void);
      void fwUartWriteFail(void);
      void gpioSetFail(void);
      void hwUartReadFail(void);
      void hwUartWriteFail(void);
      void spiReadWriteFail(void);
    private:

      // ----------------------------------------------------------------------
      // Handlers for typed from ports
      // ----------------------------------------------------------------------

      //! Handler for from_PingResponse
      //!
      void from_PingResponse_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 key /*!< Value to return to pinger*/
      );

      //! Handler for from_fpgaSpiRw
      //!
      I32 from_fpgaSpiRw_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *write_buf_ptr, 
          U8 *read_buf_ptr, 
          U32 nBytes, 
          U8 activate_slave, 
          U16 timeout, 
          U8 clock_bitrate, 
          U8 bit_order, 
          U32 delay 
      );

      //! Handler for from_spiUnclaim
      //!
      I32 from_spiUnclaim_handler(
          const NATIVE_INT_TYPE portNum /*!< The port number*/
      );

      //! Handler for from_spiClaim
      //!
      I32 from_spiClaim_handler(
          const NATIVE_INT_TYPE portNum /*!< The port number*/
      );

      //! Handler for from_spiRw8
      //!
      I32 from_spiRw8_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *write_buf_ptr, 
          U8 *read_buf_ptr, 
          U32 nWords, 
          U32 timeout_uS 
      );

      //! Handler for from_gpioSet
      //!
      I32 from_gpioSet_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_gpioClear
      //!
      I32 from_gpioClear_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_gpioRead
      //!
      U32 from_gpioRead_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_fpgaGpioSet
      //!
      I32 from_fpgaGpioSet_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_fpgaGpioClear
      //!
      I32 from_fpgaGpioClear_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_fpgaGpioRead
      //!
      U32 from_fpgaGpioRead_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 pin_num 
      );

      //! Handler for from_hwUartRx
      //!
      I32 from_hwUartRx_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *read_buf_ptr, 
          U32 nBytes, 
          U32 timeout 
      );

      //! Handler for from_hwUartTx
      //!
      I32 from_hwUartTx_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *write_buf_ptr, 
          U32 nBytes, 
          U32 timeout 
      );

      //! Handler for from_fwUartRx
      //!
      I32 from_fwUartRx_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *read_buf_ptr, 
          U32 nBytes, 
          U32 timeout 
      );

      //! Handler for from_fwUartTx
      //!
      I32 from_fwUartTx_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U8 *write_buf_ptr, 
          U32 nBytes, 
          U32 timeout 
      );

    private:

      // ----------------------------------------------------------------------
      // Helper methods
      // ----------------------------------------------------------------------

      //! Connect ports
      //!
      void connectPorts(void);

      //! Initialize components
      //!
      void initComponents(void);

    private:

      // ----------------------------------------------------------------------
      // Variables
      // ----------------------------------------------------------------------

      //! The component under test
      //!
      DriverTestComponentImpl component;

      // ----------------------------------------------------------------------
      // State variables to control test
      // ----------------------------------------------------------------------

      static const U8 UART_BYTE_VAL = 0xfe;
      GpioStateMgr m_gpio_state_mgr;
      GpioStateMgr m_fpga_gpio_state_mgr;
      bool m_hw_uart_pass;
      bool m_fw_uart_pass;
      I32 m_hw_spi_claim;
      I32 m_fw_spi_claim;
      bool m_hw_spiRw_pass;
      bool m_fw_spiRw_pass;
      U32 m_pingKey;

  };

} // end namespace App

#endif
