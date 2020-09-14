// ====================================================================== 
// \title  FatalHandlerImpl.hpp
// \author mstarch
// \brief  hpp file for FatalHandler component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 

#ifndef FatalHandler_HPP
#define FatalHandler_HPP

#include "App/FatalHandler/FatalHandlerComponentAc.hpp"
#include "fprime-sphinx-drivers/OCM/ocm.h"
//#include <Fw/Log/AmpcsEvrLogPacket.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Trap/TrapHandler.hpp>
#include <Os/Task.hpp>

#if defined(BUILD_GR712DEV) || defined(BUILD_SPHINX)
    //SYS_CLOCK_RATE 
    #include "fprime-sphinx-drivers/FPGADriver/FPGADriverComponentImpl.hpp"
    //10 Seconds
    #define APP_REBOOT_SLEEP_TIME_TICKS (SYS_CLK_RATE_HZ*10)
#endif

#define APP_FATAL_OK 0
#define APP_FATAL_OUT_OF_SPACE 1
#define APP_FATAL_CORRUPT_RECORD 2
#define APP_FATAL_SAVE_ERROR 3
#define APP_FATAL_OCM_FULL_THRESHOLD 50

#define APP_FATAL_ID_OFFSET 1
#define APP_FATAL_TIME_OFFSET 2
#define APP_FATAL_ARGS_OFFSET 4

#define APP_FATAL_RECORD_MAX_SIZE (1019/4 + 1)
#define APP_FATAL_RECORDS 10
#define APP_FATAL_TEST_BYTE_0 0xDE
#define APP_FATAL_TEST_BYTE_1 0xAD
#define APP_FATAL_TEST_BYTE_2 0xBE

// Note: This EVR ID BASE should match with Topology
#define APP_FATAL_EVR_ID_BASE 1021
#define APP_FATAL_FORCE_FATAL_EVR_ID (APP_FATAL_EVR_ID_BASE + 3)
#define APP_FATAL_DO_ASSERT_EVR_ID (APP_FATAL_EVR_ID_BASE + 4)
#define APP_FATAL_TRAP_EVR_ID (APP_FATAL_EVR_ID_BASE + 7)
#define APP_FATAL_DBE_EVR_ID (APP_FATAL_EVR_ID_BASE + 8)

//Memory bounds per device
#if defined(BUILD_SPHINX) or defined(BUILD_GR712DEV)
    #define APP_FATAL_OCM_BASE SPHINX_OCM_FATAL_BASE
    #define APP_FATAL_OCM_END  SPHINX_OCM_FATAL_END
#else
    #define APP_FATAL_OCM_BASE NULL
    #define APP_FATAL_OCM_END  NULL
#endif

#define APP_FATAL_LOOP_MAX ((SPHINX_OCM_FATAL_END - SPHINX_OCM_FATAL_BASE)/sizeof(U32) + 1)


namespace App {
  /**
   * An enumeration of traps handled by this FatalHandler
   */
  enum FATAL_TRAPS {
      DBE_DATA_ERROR = 0x09, //Uncorrectable double bit error (data access exception)
      DBE_INST_ERROR = 0x01  //Uncorrectable double bit error (instruction access exception)
  };

  class FatalHandlerComponentImpl :
    public FatalHandlerComponentBase,
    public Fw::AssertHook,
    public Fw::TrapHandler
  {

    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object FatalHandler
      //!
      FatalHandlerComponentImpl(
#if FW_OBJECT_NAMES == 1
          const char *const compName //!< The component name
#else
          void
#endif
      );
      //! Initialize the OCM pointer (for writing fatals)
      //!
      void init_ocm_pointer();

      //! Initialize object FatalHandler
      //!
      void init(
          const NATIVE_INT_TYPE queueDepth, //!< The queue depth
          const NATIVE_INT_TYPE instance = 0 //!< The instance number
      );
      //! Startup functionality
      void preamble();
      //! Destroy object FatalHandler
      //!
      ~FatalHandlerComponentImpl(void);

    PRIVATE:
      const static U16 ASSERTHOOK_MSG_MAX_SIZE = 256;
      U32 m_current;
      I8 m_destBuffer[ASSERTHOOK_MSG_MAX_SIZE];
      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for fatalReceive
      //!
      void fatalReceive_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwEventIdType Id /*!< The ID of the FATAL event*/
      );

      //! Handler implementation for PingRecv
      //!
      void PingRecv_handler(
          const NATIVE_INT_TYPE portNum, //!< The port number
          U32 key //!< Value to return to pinger
      );

      // ----------------------------------------------------------------------
      // Helper functions
      // ----------------------------------------------------------------------

      //! Handler implementation for fatalReceive
      //!
      void fatalReceive_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwEventIdType Id, /*!< The ID of the FATAL event*/
          bool immediate /*!< Flag to indicate if reboot should occur immediately or not*/
      );

    PRIVATE:
//For the LINUC build (unit-test) these need to be modifiable
//For flight, set once only
#if defined(BUILD_LINUX)
      volatile U32* m_regionStart;
      volatile U32* m_regionEnd;
      volatile U32* m_memStatus;
#else
      const volatile U32* m_regionStart;
      const volatile U32* m_regionEnd;
      const volatile U32* m_memStatus;
#endif
      Os::TaskId m_fatal_task;

      // ----------------------------------------------------------------------
      // Command handler implementations 
      // ----------------------------------------------------------------------

      //! Implementation for APP_FATAL_HDLR_FATAL command handler
      //! Command FATAL
      void APP_FATAL_HDLR_FATAL_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Implementation for APP_FATAL_HDLR_CLEAR_OCM command handler
      //! Command clear OCM Fatal buffer
      void APP_FATAL_HDLR_CLEAR_OCM_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Checks if we will not violate the region and OCM is initialized
      //! before calling saveFatalEvent function.
      //! \return: 0 on success, something else on error
      NATIVE_INT_TYPE safelySaveFatalEvent(
          FwEventIdType id, //!< Log ID
          Fw::Time &timeTag, //!< Time Tag
          Fw::LogBuffer &args //!< Buffer containing serialized log entry
      );

      //! Saves a fatal event into the on-chip memory (OCM) for downlinking
      //! when the system comes back online after reboot.
      //! \return: 0 on success, something else on error
      NATIVE_INT_TYPE saveFatalEvent(
          FwEventIdType id, //!< Log ID
          Fw::Time &timeTag, //!< Time Tag
          Fw::LogBuffer &args //!< Buffer containing serialized log entry
      );
      //! Load a fatal event from memory
      //! \return: 0 on success, something else on error
      NATIVE_INT_TYPE loadFatalEvent(
          U32* readPointer,
          FwEventIdType &id,
          Fw::Time &timeTag,
          Fw::LogBuffer &args
      );
      //! Clear enough space in our circular buffer to ensure that we can write the specified
      //! size. Updates m_current to the start of the cleared region. Space will be contiguous
      //! and thus will not wrap
      //! \param size: size to clear for
      //! \return: 0 on sucess, or something else on error
      void clearSpace();
      //! Implementation of the shared-reboot code run by the fatal command
      //! and fatal event items
      //! \param immediate: (optional) reboot immediately? Default: false
      //! Note: for certain errors, e.g. DBEs, software is likely defunct
      //        and it should be rebooted immediately
      void rebootCraft(bool immediate = false);

      // ----------------------------------------------------------------------
      // Trap function
      // ----------------------------------------------------------------------
      //! Handles incoming traps in the trap handler, logs a failure EVR, and
      //! then forces a reboot. Rebooted immediately to prevent cascading
      //! failures.
      //! \param U32 trap: trap number that occurred
      //!
      void doTrap(U32 trap);

      // ----------------------------------------------------------------------
      // AssertHook functions
      // ----------------------------------------------------------------------
      void reportAssert(
              FILE_NAME_ARG file,
              NATIVE_UINT_TYPE lineNo,
              NATIVE_UINT_TYPE numArgs,
              AssertArg arg1,
              AssertArg arg2,
              AssertArg arg3,
              AssertArg arg4,
              AssertArg arg5,
              AssertArg arg6
              );
      void printAssert(const I8* msg);
      void doAssert(void);
    };

} // end namespace App

#endif
