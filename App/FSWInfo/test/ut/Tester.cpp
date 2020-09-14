// ======================================================================
// \title  FSWInfo.hpp
// \author vwong
// \brief  cpp file for FSWInfo test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================

#include "Tester.hpp"
#include "Fw/Com/ComPacket.hpp"

#define INSTANCE 0
#define CMD_SEQ 0
#define MAX_HISTORY_SIZE 10
#define QUEUE_DEPTH 10

#define FSWINFO_UT_REG 0x4000

namespace App {

  // ----------------------------------------------------------------------
  // Construction and destruction
  // ----------------------------------------------------------------------

  Tester ::
    Tester(void) :
#if FW_OBJECT_NAMES == 1
      FSWInfoGTestBase("Tester", MAX_HISTORY_SIZE),
      component("FSWInfo"),
      pingKey(0)
#else
      FSWInfoGTestBase(MAX_HISTORY_SIZE),
      component(),
      pingKey(0)
#endif
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
    testPing(void) 
  {
    FW_ASSERT(this->pingKey == 0);
    U32 key = 0xA3BAC362;
    this->invoke_to_PingRecv(0, key);
    this->component.doDispatch();
    ASSERT_EQ(this->pingKey, key);
  }

  void Tester ::
    preambleEmitsVersionEVR(void)
  {
    this->component.preamble();
    ASSERT_EVENTS_SIZE(2);
    ASSERT_EVENTS_FSWVersion_SIZE(1);
    ASSERT_EVENTS_FirmwareVersion_SIZE(1);
  }

  void Tester ::
    reportFSWInfoPortEmitsEVR(void)
  {
    this->invoke_to_reportFSWInfo(0);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(2);
    ASSERT_EVENTS_FSWVersion_SIZE(1);
    ASSERT_EVENTS_FirmwareVersion_SIZE(1);
  }

  void Tester ::
    reportFSWInfoCommandEmitsEVR(void)
  {
    this->sendCmd_FSWINFO_DMP(INSTANCE, CMD_SEQ);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(2);
    ASSERT_EVENTS_FSWVersion_SIZE(1);
    ASSERT_EVENTS_FirmwareVersion_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_DMP, CMD_SEQ, Fw::COMMAND_OK);
  }

  void Tester ::
    handlesInitSRAMCommand(void)
  {
    this->sendCmd_FSWINFO_INIT_SRAM_VALS(INSTANCE, CMD_SEQ);
    this->component.doDispatch();
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_INIT_SRAM_VALS, CMD_SEQ, Fw::COMMAND_OK);
  }

  void Tester ::
    handlesResetFSWCommand(void)
  {
    this->sendCmd_FSWINFO_RESET_FSW(INSTANCE, CMD_SEQ);
    this->component.doDispatch();
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_RESET_FSW, CMD_SEQ, Fw::COMMAND_OK);
  }

  void Tester ::
    handlesTimeCommands(void)
  {
    this->clearHistory();

    this->sendCmd_FSWINFO_TIME_UPDT_REL(INSTANCE, CMD_SEQ, 10, 10);
    this->component.doDispatch();
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_TIME_UPDT_REL, CMD_SEQ, Fw::COMMAND_EXECUTION_ERROR);

    this->clearHistory();

    this->sendCmd_FSWINFO_TIME_UPDT_ABS(INSTANCE, CMD_SEQ, 100, 100);
    this->component.doDispatch();
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_TIME_UPDT_ABS, CMD_SEQ, Fw::COMMAND_EXECUTION_ERROR);
  }

  void Tester ::
    handlesFPGAWdogTimeLeftDmpCommand(void)
  {
    this->sendCmd_FSWINFO_FPGA_WDOG_TIME_LEFT_DMP(INSTANCE, CMD_SEQ);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FPGAWatchDogTimeLeft_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_FPGA_WDOG_TIME_LEFT_DMP, CMD_SEQ, Fw::COMMAND_OK);
  }

  void Tester ::
    handlesReadRegCommand(void)
  {
    // setup register to read from
    U32 regVal = 0x2000;
    //initial register values, set as desired
    Drv::SphinxDrvReg::fileEntry fileRegs[] = {
        {regVal, FSWINFO_UT_REG}
    };

    //currently set to zero to not interfere with other reads/writes
    //change to 5 to initialize the above register with values
    Drv::SphinxDrvReg regData(fileRegs, 1);
    regData.writeFile();

    this->sendCmd_FSWINFO_READ_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG, 0, 255);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_ReadReg_SIZE(1);
    ASSERT_EVENTS_FSWInfo_ReadReg(0, FSWINFO_UT_REG, regVal);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_READ_REG, CMD_SEQ, Fw::COMMAND_OK);

    this->clearHistory();
    this->sendCmd_FSWINFO_READ_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG, 0, 250);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_LockKey_Mismatch_SIZE(1);
    ASSERT_EVENTS_FSWInfo_LockKey_Mismatch(0, 0, 250);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_READ_REG, CMD_SEQ, Fw::COMMAND_VALIDATION_ERROR);

    this->clearHistory();
    this->sendCmd_FSWINFO_READ_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG+1, 0, 255);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_Address_Misaligned_SIZE(1);
    ASSERT_EVENTS_FSWInfo_Address_Misaligned(0, FSWINFO_UT_REG+1);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_READ_REG, CMD_SEQ, Fw::COMMAND_VALIDATION_ERROR);
  }

  void Tester ::
    handlesWriteRegCommand(void)
  {
    U32 val = 0x555;
    this->sendCmd_FSWINFO_WRITE_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG, val, 0, 255);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_WriteRegSucc_SIZE(1);
    ASSERT_EVENTS_FSWInfo_WriteRegSucc(0, FSWINFO_UT_REG, val);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_WRITE_REG, CMD_SEQ, Fw::COMMAND_OK);

    this->clearHistory();
    this->sendCmd_FSWINFO_WRITE_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG, val, 0, 250);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_LockKey_Mismatch_SIZE(1);
    ASSERT_EVENTS_FSWInfo_LockKey_Mismatch(0, 0, 250);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_WRITE_REG, CMD_SEQ, Fw::COMMAND_VALIDATION_ERROR);

    this->clearHistory();
    this->sendCmd_FSWINFO_WRITE_REG(INSTANCE, CMD_SEQ, FSWINFO_UT_REG+1, val, 0, 255);
    this->component.doDispatch();
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_FSWInfo_Address_Misaligned_SIZE(1);
    ASSERT_EVENTS_FSWInfo_Address_Misaligned(0, FSWINFO_UT_REG+1);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, FSWInfoComponentBase::OPCODE_FSWINFO_WRITE_REG, CMD_SEQ, Fw::COMMAND_VALIDATION_ERROR);
  }

  // ----------------------------------------------------------------------
  // Handlers for typed from ports
  // ----------------------------------------------------------------------

  void Tester ::
    from_timeREL_handler(
        const NATIVE_INT_TYPE portNum,
        U32 SC_Time_Secs,
        U32 SC_Time_uSecs
    )
  {
    this->pushFromPortEntry_timeREL(SC_Time_Secs, SC_Time_uSecs);
  }

  void Tester ::
    from_PingResponse_handler(
        const NATIVE_INT_TYPE portNum,
        U32 key
    )
  {
    this->pushFromPortEntry_PingResponse(key);
    this->pingKey = key;
  }

  void Tester ::
    from_cmdOut_handler(
        const NATIVE_INT_TYPE portNum,
        Fw::ComBuffer &data,
        U32 context
    )
  {
    this->pushFromPortEntry_cmdOut(data, context);
  }

  void Tester ::
    from_timeABS_handler(
        const NATIVE_INT_TYPE portNum,
        U32 SC_Time_Secs,
        U32 SC_Time_uSecs
    )
  {
    this->pushFromPortEntry_timeABS(SC_Time_Secs, SC_Time_uSecs);
  }

  // ----------------------------------------------------------------------
  // Helper methods
  // ----------------------------------------------------------------------

  void Tester ::
    connectPorts(void)
  {

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

    // reportFSWInfo
    this->connect_to_reportFSWInfo(
        0,
        this->component.get_reportFSWInfo_InputPort(0)
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

    // LogText
    this->component.set_LogText_OutputPort(
        0,
        this->get_from_LogText(0)
    );

    // PingRecv
    this->connect_to_PingRecv(
        0,
        this->component.get_PingRecv_InputPort(0)
    );

    // PingResponse
    this->component.set_PingResponse_OutputPort(
        0,
        this->get_from_PingResponse(0)
    );

    // cmdOut
    this->component.set_cmdOut_OutputPort(
        0, 
        this->get_from_cmdOut(0)
    );

    // timeABS
    this->component.set_timeABS_OutputPort(
        0, 
        this->get_from_timeABS(0)
    );

    // timeREL
    this->component.set_timeREL_OutputPort(
        0, 
        this->get_from_timeREL(0)
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
