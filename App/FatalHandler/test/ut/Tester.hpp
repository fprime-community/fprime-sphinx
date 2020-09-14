// ====================================================================== 
// \title  FatalHandler/test/ut/Tester.hpp
// \author mstarch
// \brief  hpp file for FatalHandler test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "App/FatalHandler/FatalHandlerComponentImpl.hpp"

#define APP_COMMON_FATAL_TEST_CANARY 0xdeadbeef

namespace App {

  class Tester :
    public FatalHandlerGTestBase
  {

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

      //! To do
      //!
      void clear_memory();
      void clear_space(void);
      void full_warning(void);
      void fatal_log(void);
      void fatal_overflow_drop(void);
      void small_region_drop(void);
      void ping_test(void);
      void memory_not_initialized(void);
      void memory_corrupt(void);

      //Helpers
      void check_downlinked_evr(NATIVE_INT_TYPE index);

    private:

      // ----------------------------------------------------------------------
      // Handlers for typed from ports
      // ----------------------------------------------------------------------

      //! Handler for from_storedEvrOut
      //!
      void from_storedEvrOut_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwEventIdType id, /*!< Log ID*/
          Fw::Time &timeTag, /*!< Time Tag*/
          Fw::LogSeverity severity, /*!< The severity argument*/
          Fw::LogBuffer &args /*!< Buffer containing serialized log entry*/
      );

      //! Handler for from_PingResponse
      //!
      void from_PingResponse_handler(
          const NATIVE_INT_TYPE portNum, //!< The port number
          U32 key //!< Value to return to pinger
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
      FatalHandlerComponentImpl component;

  };

} // end namespace App

#endif
