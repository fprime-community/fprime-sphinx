// ======================================================================
// \title  FatalHandlerImpl.cpp
// \author mstarch
// \brief  cpp file for FatalHandler component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================

#include <App/FatalHandler/FatalHandlerComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include <fprime-sphinx-drivers/Util/SphinxDrvUtil.hpp>
#if defined(BUILD_SPHINX) or defined(BUILD_GR712DEV)
extern "C" {
#include <rebootLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <intLib.h>
}
#endif
#include <App/IdleTask/IdleTaskComponentImpl.hpp> //For DBE failing address
#include <cstring>
namespace App {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  FatalHandlerComponentImpl ::
#if FW_OBJECT_NAMES == 1
    FatalHandlerComponentImpl(
        const char *const compName
    ) :
      FatalHandlerComponentBase(compName),
#else
    FatalHandlerImpl(void),
#endif
    Fw::AssertHook(),
    Fw::TrapHandler(),
    m_current(0),
    m_regionStart((U32*)APP_FATAL_OCM_BASE),
    m_regionEnd((U32*)APP_FATAL_OCM_END),
    m_memStatus((U32*)SPHINX_OCM_VALID_FLAG_ADDR),
    m_fatal_task()
  {
  }

  void FatalHandlerComponentImpl ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    FatalHandlerComponentBase::init(queueDepth, instance);
    //Checking for m_current should be done on init, after constructor
    U32 val = (*this->m_memStatus);
    if (val != SPHINX_OCM_VALID_FLAG_VALUE) {
        return;
    }
    init_ocm_pointer();
  }

  void FatalHandlerComponentImpl :: init_ocm_pointer() {
    U32* iterator = NULL;
    this->m_current = 0;
    while (this->m_current < APP_FATAL_RECORDS) {
        //Found an empty space, stop here
        iterator = this->m_current * APP_FATAL_RECORD_MAX_SIZE + (U32*)this->m_regionStart;
        if (iterator + APP_FATAL_RECORD_MAX_SIZE > this->m_regionEnd || *iterator == 0) {
          break;
        }
        this->m_current = this->m_current + 1;
    }
  }

  void FatalHandlerComponentImpl :: preamble() {
      U32 id;
      U32 percentage = 0;
      U32 number;
      NATIVE_INT_TYPE status; 
      Fw::Time timeTag;
      Fw::LogBuffer args;
      m_fatal_task = Os::Task::getOsIdentifier();
      //Prevents access before OCM is initialized
      U32 val = (*this->m_memStatus);
      if (val != SPHINX_OCM_VALID_FLAG_VALUE) {
          this->log_WARNING_HI_AppFatalHandler_OCM_NOT_INITIALIZED();
          return;
      }
      for (number = 0; number < this->m_current && number < APP_FATAL_RECORDS; number += 1)
      {
          status = loadFatalEvent((number * APP_FATAL_RECORD_MAX_SIZE) + (U32*)this->m_regionStart, id, timeTag, args);
          if(status == 0){
            this->storedEvrOut_out(0, id, timeTag, Fw::LOG_FATAL, args);
          } else if (status == APP_FATAL_CORRUPT_RECORD) {
            this->log_WARNING_HI_AppFatalHandler_OcmInvalidSize();
          }
      }
      //The iterator variable must be equivalent to the current "next" after downlink
      FW_ASSERT(number == this->m_current);
      percentage = (100 * this->m_current)/APP_FATAL_RECORDS;
      //Alert if more than 50% full
      if (percentage >= APP_FATAL_OCM_FULL_THRESHOLD) {
          this->log_WARNING_HI_AppFatalHandler_OcmFull(percentage);
      }
  }

  FatalHandlerComponentImpl ::
    ~FatalHandlerComponentImpl(void)
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void FatalHandlerComponentImpl ::
    fatalReceive_handler(
        const NATIVE_INT_TYPE portNum,
        FwEventIdType Id
    )
  {
    (void) fatalReceive_handler(portNum, Id, true);
  }

  void FatalHandlerComponentImpl ::
    fatalReceive_handler(
        const NATIVE_INT_TYPE portNum,
        FwEventIdType Id,
        bool immediate
    )
  {
    this->rebootCraft(immediate);
  }

  void FatalHandlerComponentImpl ::
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

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void FatalHandlerComponentImpl ::
    APP_FATAL_HDLR_FATAL_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    Fw::SerializeStatus status = Fw::FW_SERIALIZE_OK;
    Fw::Time curr = this->getTime();
    Fw::LogBuffer args = Fw::LogBuffer();

    status = args.serialize((U8)APP_FATAL_TEST_BYTE_0);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
    status = args.serialize((U8)APP_FATAL_TEST_BYTE_1);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
    status = args.serialize((U8)APP_FATAL_TEST_BYTE_2);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

    if(APP_FATAL_OK != this->safelySaveFatalEvent(APP_FATAL_FORCE_FATAL_EVR_ID, curr, args))
    {
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
      return;
    }

    //Log a fatal event and add some parameters
    this->log_FATAL_AppFatalHandler_ForcedFatal(
            (U8)APP_FATAL_TEST_BYTE_0,
            (U8)APP_FATAL_TEST_BYTE_1,
            (U8)APP_FATAL_TEST_BYTE_2);
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  void FatalHandlerComponentImpl ::
    APP_FATAL_HDLR_CLEAR_OCM_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    this->clearSpace();
    this->log_ACTIVITY_HI_AppFatalHandler_OcmClear();
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  // ----------------------------------------------------------------------
  // Helper functions
  // -------

  NATIVE_INT_TYPE FatalHandlerComponentImpl ::
    loadFatalEvent(
        U32* readPointer,
        FwEventIdType &id,
        Fw::Time &timeTag,
        Fw::LogBuffer &args
    )
  {
    U32 i;
    U32 copySize;
    U32 size;
    FW_ASSERT(this->m_regionStart <= readPointer && readPointer < this->m_regionEnd);
    size = *readPointer;
    //Size is corrupt if it is less than the header size, or greater than max record size
    if (size < (sizeof(U32) * (APP_FATAL_ARGS_OFFSET - 1)) || (size > APP_FATAL_RECORD_MAX_SIZE)) {
        return APP_FATAL_CORRUPT_RECORD;
    }
    id = *(readPointer + APP_FATAL_ID_OFFSET);
    timeTag.set(*(readPointer + APP_FATAL_TIME_OFFSET), *(readPointer + APP_FATAL_TIME_OFFSET + 1));
    //Subtract off size
    size = size - (sizeof(U32) * (APP_FATAL_ARGS_OFFSET - 1));
    //Loop through each set of U32 bytes
    for (i = 0; i < size && i < args.getBuffCapacity(); i += sizeof(U32))
    {
      copySize = size - i;
      copySize = (copySize > sizeof(U32)) ? sizeof(U32) : copySize;
      memcpy(args.getBuffAddr() + i, readPointer + APP_FATAL_ARGS_OFFSET + i/sizeof(U32), copySize);
    }
    Fw::SerializeStatus status = args.setBuffLen(size);
    FW_ASSERT(Fw::FW_SERIALIZE_OK == status);
    return 0;
  }

  NATIVE_INT_TYPE FatalHandlerComponentImpl ::
    safelySaveFatalEvent(
        FwEventIdType id,
        Fw::Time &timeTag,
        Fw::LogBuffer &args
    )
  {
    NATIVE_INT_TYPE status = APP_FATAL_SAVE_ERROR;

    //Write out Fatal messages if we will not violate the region and OCM is initialized
    U32* location = (this->m_current * APP_FATAL_RECORD_MAX_SIZE) + (U32*)this->m_regionStart;
    U32 val = (*this->m_memStatus);
    if (location + APP_FATAL_RECORD_MAX_SIZE <= this->m_regionEnd && val == SPHINX_OCM_VALID_FLAG_VALUE) {
        //Device is rebooting, ignoring the failure to save a fatal
        status = saveFatalEvent(id, timeTag, args);
    }

    return status;
  }

  NATIVE_INT_TYPE FatalHandlerComponentImpl ::
    saveFatalEvent(
        FwEventIdType id,
        Fw::Time &timeTag,
        Fw::LogBuffer &args
    )
  {
    U32 i = 0;
    U32 copySize = 0;
    U32 value = 0;
    U32* writePointer = (this->m_current * APP_FATAL_RECORD_MAX_SIZE) + (U32*)this->m_regionStart;
    //Error if full
    if (this->m_current >= APP_FATAL_RECORDS || (writePointer + APP_FATAL_RECORD_MAX_SIZE) > this->m_regionEnd) {
        return APP_FATAL_OUT_OF_SPACE;
    }
    FW_ASSERT(this->m_regionStart <= writePointer && writePointer < this->m_regionEnd);
    //Write header bytes
    *(writePointer) = (APP_FATAL_ARGS_OFFSET - 1) * sizeof(U32) + args.getBuffLength();
    *(writePointer + APP_FATAL_ID_OFFSET) = id;
    *(writePointer + APP_FATAL_TIME_OFFSET) = timeTag.getSeconds();
    *(writePointer + APP_FATAL_TIME_OFFSET + 1) = timeTag.getUSeconds();
    //Loop and write out integers
    for (i = 0; (i*sizeof(U32)) < args.getBuffLength() && i < (APP_FATAL_RECORD_MAX_SIZE - APP_FATAL_ARGS_OFFSET); i++)
    {
      //Clear the value, then copy overtop just the bytes
      value = 0;
      copySize = args.getBuffLength() - (i * sizeof(U32));
      copySize = (copySize > sizeof(U32)) ? sizeof(U32) : copySize;
      memcpy((void *)&value, args.getBuffAddr() + (i * sizeof(U32)), copySize);
      *(writePointer + APP_FATAL_ARGS_OFFSET + i) = value;
    }
    //Assert that recorded size is big enough for the header
    FW_ASSERT(*writePointer >= (sizeof(U32) * (APP_FATAL_ARGS_OFFSET - 1)));
    //Update current to wrap
    this->m_current += 1;
    return APP_FATAL_OK;
  }
  void FatalHandlerComponentImpl ::
    clearSpace()
  {
    U32 i = 0;
    //Walk through every integer and set to zero
    FW_ASSERT(this->m_regionStart < this->m_regionEnd);
    U32* clearable = (U32*)this->m_regionStart;
    while ((clearable < (U32*)this->m_regionEnd) && (i < APP_FATAL_LOOP_MAX))
    {
        *(clearable) = 0;
        clearable += 1;
        i++;
    }
    this->m_current = 0;
  }

  void FatalHandlerComponentImpl ::
    rebootCraft(bool immediate)
  {
#if defined(BUILD_SPHINX) or defined(BUILD_GR712DEV)
    // Delay for x seconds if not immediately
    if (!immediate) {
        taskDelay(APP_REBOOT_SLEEP_TIME_TICKS);
    }
    // reboot
    reboot(BOOT_NORMAL);
#endif
  }

  // Framework trap handler
  // Please note, this function should not ASSERT.
  // Hence the lack of return-value checking (e.g. args.serialize)
  void FatalHandlerComponentImpl ::
    doTrap(U32 trap)
  {
    U32 evr = 0;
    U32 arg_count = 2; //Stack trace, trap
    Fw::Time tag = this->getTime();
    Fw::LogBuffer args = Fw::LogBuffer();
    //Switch on the trap, and add trap specific arguments
    switch (trap) {
        case DBE_INST_ERROR:
        case DBE_DATA_ERROR:
            arg_count++;
            break;
        default:
            break;
    }
    (void)args.serialize(trap);
    //Check for handled traps
    switch (trap) {
        //Handle the double-bit errors
        case DBE_INST_ERROR:
        case DBE_DATA_ERROR:
            {
                //Store address as part of the EVR
                U32 addr = Drv::readReg(IDLE_TASK_AHB_FAILING_ERROR);
                evr = APP_FATAL_DBE_EVR_ID;
                (void)args.serialize(addr);
                //If in OCM, rewrite the address to "wash"
                //the EDAC bits to prevent reboot cycle
                if ((addr >= SPHINX_OCM_BASE_ADDR) && (addr < SPHINX_OCM_HIGH_ADDR)) {
                     Drv::writeReg(addr, 0);
                }
            }
            break;
        //Any other trap
        default:
            evr = APP_FATAL_TRAP_EVR_ID;
            break;
    }
    //Serialize the arguments
    
    //Fatal handler may not be up and running, force (re)init
    init_ocm_pointer();
    //Call the fatal handler
    (void)safelySaveFatalEvent(evr, tag, args);
    fatalReceive_handler(0, evr, true);
  }

#if FW_ASSERT_LEVEL == FW_NO_ASSERT

#else

#if FW_ASSERT_LEVEL == FW_FILEID_ASSERT
#define fileIdFs "Assert file ID 0x%08X: Line: %d "
#else
#define fileIdFs "Assert file \"%s\": Line: %d "
#endif

  void FatalHandlerComponentImpl ::
    reportAssert(
          FILE_NAME_ARG file,
          NATIVE_UINT_TYPE lineNo,
          NATIVE_UINT_TYPE numArgs,
          AssertArg arg1,
          AssertArg arg2,
          AssertArg arg3,
          AssertArg arg4,
          AssertArg arg5,
          AssertArg arg6
          )
  {
    NATIVE_INT_TYPE buffSize = sizeof(m_destBuffer);
    switch (numArgs) {
        case 0:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs,file,lineNo);
            break;
        case 1:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d",file,lineNo,
                    arg1);
            break;
        case 2:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d %d",file,lineNo,
                    arg1,arg2);
            break;
        case 3:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d %d %d",file,lineNo,
                    arg1,arg2,arg3);
            break;
        case 4:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d %d %d %d",file,lineNo,
                    arg1,arg2,arg3,arg4);
            break;
        case 5:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d %d %d %d %d",file,lineNo,
                    arg1,arg2,arg3,arg4,arg5);
            break;
        case 6:
            (void)snprintf((char*)m_destBuffer,buffSize,fileIdFs "%d %d %d %d %d %d",file,lineNo,
                    arg1,arg2,arg3,arg4,arg5,arg6);
            break;
        default: // in an assert already, what can we do?
            break;
    }

    // null terminate
    m_destBuffer[buffSize-1] = 0;

  }

  void FatalHandlerComponentImpl ::
    printAssert(const I8* msg)
  {
    /* Do nothing here */
  }

  void FatalHandlerComponentImpl ::
    doAssert(void)
  {
    static U8 assert_count = 2;
    //Prevents asserts triggering asserts triggering asserts
    //Will catch an assert, and a single assert it triggers
    if (assert_count > 0) {
        Fw::LogBuffer args = Fw::LogBuffer();
        Fw::Time curr = this->getTime();
        assert_count = assert_count - 1;
        Fw::LogStringArg assert_msg((char*)m_destBuffer);

        (void)args.serialize(assert_msg);
        this->log_FATAL_AppFatalHandler_DO_ASSERT(assert_msg);
        (void)safelySaveFatalEvent(APP_FATAL_DO_ASSERT_EVR_ID, curr, args);

        //On VxWorks, if a task Asserts, kill the task to prevent
        //it from doing something stupid.
        //However, it would be stupid to kill the FatalHandler
        //so prevent that too.
        //Also, per the vxworks documentation, taskExit during
        //interrupt context is VERY stupid, so don't do that.
        #ifdef TGT_OS_TYPE_VXWORKS
            if (!intContext() && Os::Task::getOsIdentifier() != m_fatal_task) {
                (void) taskUnlock(); //Prevent stupid
                ::taskExit(1);
            }
        #endif
        assert_count = assert_count + 1;
    }
  }

#endif /* end FW_ASSERT_LEVEL == FW_NO_ASSERT */

} // end namespace App
