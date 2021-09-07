// ======================================================================
// \title  FSWInfo/test/ut/Tester.hpp
// \author vwong
// \brief  hpp file for FSWInfo test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "App/FSWInfo/FSWInfoComponentImpl.hpp"
#include "fprime-sphinx-drivers/Util/SphinxDrvReg.hpp"

namespace App {

  class Tester :
    public FSWInfoGTestBase
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


      void testPing(void);
      void preambleEmitsVersionEVR(void);
      void reportFSWInfoPortEmitsEVR(void);
      void reportFSWInfoCommandEmitsEVR(void);
      void handlesInitSRAMCommand(void);
      void handlesResetFSWCommand(void);
      void handlesTimeCommands(void);
      void handlesFPGAWdogTimeLeftDmpCommand(void);
      void handlesReadRegCommand(void);
      void handlesWriteRegCommand(void);

    private:

      // ----------------------------------------------------------------------
      // Handlers for typed from ports
      // ----------------------------------------------------------------------

      //! Handler for from_timeREL
      //!
      void from_timeREL_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 SC_Time_Secs, 
          U32 SC_Time_uSecs 
      );

      //! Handler for from_PingResponse
      //!
      void from_PingResponse_handler(
          const NATIVE_INT_TYPE portNum, //!< The port number
          U32 key //!< Value to return to pinger
      );

      //! Handler for from_cmdOut
      //!
      void from_cmdOut_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::ComBuffer &data, /*!< Buffer containing packet data*/
          U32 context /*!< Call context value; meaning chosen by user*/
      );

      //! Handler for from_timeABS
      //!
      void from_timeABS_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          U32 SC_Time_Secs, 
          U32 SC_Time_uSecs 
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
      FSWInfoComponentImpl component;

      //! Ping signal
      U32 pingKey;

  };

} // end namespace App

#endif
