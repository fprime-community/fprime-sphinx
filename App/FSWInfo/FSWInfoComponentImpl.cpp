// ======================================================================
// \title  FSWInfoImpl.cpp
// \author vwong
// \brief  cpp file for FSWInfo component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================


#include <App/FSWInfo/FSWInfoComponentImpl.hpp>
#include "Fw/Types/BasicTypes.hpp"

extern "C" {
#ifdef TGT_OS_TYPE_VXWORKS
#include <sysLib.h>
#include <rebootLib.h>
#endif
#include <fprime-sphinx-drivers/OCM/ocm.h>
}
#include <fprime-sphinx-drivers/Util/SphinxDrvUtil.hpp>

namespace App {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  FSWInfoComponentImpl ::
#if FW_OBJECT_NAMES == 1
    FSWInfoComponentImpl(
        const char *const compName
    ) :
      FSWInfoComponentBase(compName)
#else
    FSWInfoImpl(void)
#endif
  {
  }

  void FSWInfoComponentImpl ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    FSWInfoComponentBase::init(queueDepth, instance);
  }

  FSWInfoComponentImpl ::
    ~FSWInfoComponentImpl(void)
  {
  }

  void FSWInfoComponentImpl ::
    preamble(void)
  {
    /* NOTE: SRAM partition flags are reset in Topology */
    //resetSRAMPartitionFlags();
    reportFSWInfo();
  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void FSWInfoComponentImpl ::
    PingRecv_handler(
        const NATIVE_INT_TYPE portNum,
        U32 key
    )
  {
    if (this-> isConnected_PingResponse_OutputPort(0))
    {
        this->PingResponse_out(0, key);
    }
  }


  void FSWInfoComponentImpl ::
    reportFSWInfo_handler(
        const NATIVE_INT_TYPE portNum
    )
  {
    reportFSWInfo();
  }

  void FSWInfoComponentImpl ::
    schedIn_handler(
        const NATIVE_INT_TYPE portNum,
        NATIVE_UINT_TYPE context
    )
  {

  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void FSWInfoComponentImpl ::
    FSWINFO_DMP_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    reportFSWInfo();
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  void FSWInfoComponentImpl ::
    FSWINFO_RESET_FSW_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // Need to write specific value to particular register in order
    // to indicate to firmware we are doing a soft-reboot
    Drv::writeReg(SPHINX_SOFT_RESET_REG, SPHINX_SOFT_RESET_VAL);

#ifdef TGT_OS_TYPE_VXWORKS
    reboot(BOOT_NORMAL);
#endif
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  void FSWInfoComponentImpl ::
    FSWINFO_INIT_SRAM_VALS_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    initSRAM();
    resetSRAMPartitionFlags();
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  void FSWInfoComponentImpl ::
    FSWINFO_TIME_UPDT_ABS_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 secs,
        U32 usecs
    )
  {
    Fw::Time curr_time;

    curr_time = this->getTime();

    // New absolute commanded time whole
    // seconds must be larger than
    // current time whole second value.
    if (curr_time.getSeconds() >= secs) {
      // Do not update time
      this->log_WARNING_HI_NewTime_ABS_Invalid(secs, curr_time.getSeconds());
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
      return;
    }

    if (this->isConnected_timeABS_OutputPort(0)) {
#if defined(BUILD_SPHINX)
      this->timeABS_out(0, secs, usecs); 
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
#else
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
#endif
    }
    else {
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void FSWInfoComponentImpl ::
    FSWINFO_TIME_UPDT_REL_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 secs,
        U32 usecs
    )
  {
    if (this->isConnected_timeABS_OutputPort(0)) {
#if defined(BUILD_SPHINX)
      this->timeREL_out(0, secs, usecs); 
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
#else
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
#endif
    }
    else {
      this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
    }
  }

  void FSWInfoComponentImpl ::
    FSWINFO_FPGA_WDOG_TIME_LEFT_DMP_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    U32 val = Drv::readReg(SPHINX_FPGA_WDOG_TIME_LEFT_REG);
    this->log_ACTIVITY_HI_FPGAWatchDogTimeLeft(val); 
    this->cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
  }

  void FSWInfoComponentImpl ::
    FSWINFO_READ_REG_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 address,
        U8 lock,
        U8 key
    )
  {
      // check lock and key
      U8 invKey = ~key;
      if ((U8)lock != invKey) {
          this->log_WARNING_HI_FSWInfo_LockKey_Mismatch(lock, key);
          this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
          return;
      }

      // Check for 4-byte aligned address
      if ((address % 0x4) != 0) {
          this->log_WARNING_HI_FSWInfo_Address_Misaligned(address);
          this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
          return;
      }

      // Read value at 4-byte address
      U32 val = Drv::readReg(address);

      this->log_ACTIVITY_HI_FSWInfo_ReadReg(address, val);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
  }

  void FSWInfoComponentImpl ::
    FSWINFO_WRITE_REG_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 address,
        U32 value,
        U8 lock,
        U8 key
    )
  {
      // check lock and key
      U8 invKey = ~key;
      if ((U8)lock != invKey) {
          this->log_WARNING_HI_FSWInfo_LockKey_Mismatch(lock, key);
          this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
          return;
      }

      // Check for 4-byte aligned address
      if ((address % 0x4) != 0) {
          this->log_WARNING_HI_FSWInfo_Address_Misaligned(address);
          this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);
          return;
      }

      // Write value at 4-byte address
      Drv::writeReg(address, value);

      // Read back value at 4-byte address
      U32 currVal = Drv::readReg(address);

      if (currVal != value) {
        this->log_WARNING_HI_FSWInfo_WriteRegFail(address, value, currVal);
        this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_EXECUTION_ERROR);
      }
      else {
        this->log_ACTIVITY_HI_FSWInfo_WriteRegSucc(address, value);
        this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);
      }
  }

  // ----------------------------------------------------------------------
  // Private supporting functions
  // ----------------------------------------------------------------------

  void FSWInfoComponentImpl::reportFSWInfo(void)
  {
    // OCM boot count
    U32 sramFlag = Drv::readReg(SPHINX_OCM_VALID_FLAG_ADDR);
    if (sramFlag == SPHINX_OCM_VALID_FLAG_VALUE) {
      U32 bootCount = Drv::readReg(SPHINX_OCM_BOOT_COUNT_ADDR);
      tlmWrite_FSWInfo_SRAMBootCount(bootCount);
    }

    // FSW Version
    Fw::LogStringArg msg(FSW_VERSION);
    this->log_ACTIVITY_LO_FSWVersion(msg);

    // Sphinx firmware version
    U32 fwVer = Drv::readReg(SPHINX_FW_VERSION_REG);
    this->log_ACTIVITY_LO_FirmwareVersion(fwVer);
  }

  void FSWInfoComponentImpl::initSRAM(void)
  {
    Drv::writeReg(SPHINX_OCM_VALID_FLAG_ADDR, SPHINX_OCM_VALID_FLAG_VALUE);
    Drv::writeReg(SPHINX_OCM_BOOT_COUNT_ADDR, 0);

#ifdef TGT_OS_TYPE_VXWORKS
    //Prep OCM for fatal storage
    FW_ASSERT(SPHINX_OCM_FATAL_BASE < SPHINX_OCM_FATAL_END);
    U32* clearable = (U32*)SPHINX_OCM_FATAL_BASE;
    while (clearable < (U32*)SPHINX_OCM_FATAL_END)
    {
        *(clearable) = 0;
        clearable += 1;
    }
#endif
  }

  void FSWInfoComponentImpl::resetSRAMPartitionFlags(void)
  {
    Drv::writeReg(SPHINX_OCM_UTIL_PART_FLAG_ADDR, 0);
    Drv::writeReg(SPHINX_OCM_ENG_PART_FLAG_ADDR, 0);
    Drv::writeReg(SPHINX_OCM_SCI_PART_FLAG_ADDR, 0);
    Drv::writeReg(SPHINX_OCM_SEQ_PART_FLAG_ADDR, 0);
  }

} // end namespace App
