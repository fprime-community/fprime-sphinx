// ====================================================================== 
// \title  FatalHandler.hpp
// \author mstarch
// \brief  cpp file for FatalHandler test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 

#include "Tester.hpp"

#define INSTANCE 0
#define QUEUE_DEPTH 10
#define MAX_HISTORY_SIZE (QUEUE_DEPTH + 12 + 1)
#define LOG_ARG_OFFSET 6
#define PING_KEY 123

//Must use global scope, as it needs to be available to our tested object
U32 TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS + 1];
U32 TEST_MEMORY_STATUS = SPHINX_OCM_VALID_FLAG_VALUE;

namespace App {

  // ----------------------------------------------------------------------
  // Construction and destruction 
  // ----------------------------------------------------------------------

  Tester ::
    Tester(void) : 
#if FW_OBJECT_NAMES == 1
      FatalHandlerGTestBase("Tester", MAX_HISTORY_SIZE),
      component("FatalHandler")
#else
      FatalHandlerGTestBase(MAX_HISTORY_SIZE),
      component()
#endif
  {
    //Initialize the memory before initializing component
    component.m_regionStart = &TEST_MEMORY[0];
    component.m_regionEnd = &TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS];
    component.m_memStatus = &TEST_MEMORY_STATUS;
    //Set a value to detect buffer overflow
    TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS] = APP_COMMON_FATAL_TEST_CANARY;
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

  void Tester :: clear_space(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    ASSERT_EQ(APP_COMMON_FATAL_TEST_CANARY, TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS]);
    //Set memory to be something not zero
    memset(TEST_MEMORY, 0xFF, sizeof(TEST_MEMORY-sizeof(U32)));
    this->sendCmd_APP_FATAL_HDLR_CLEAR_OCM(0, 0);
    this->component.doDispatch();
    for (i = 0; i < APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS; i++) {
      ASSERT_EQ(TEST_MEMORY[i], 0);
    }
    ASSERT_EQ(APP_COMMON_FATAL_TEST_CANARY, TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS]);
    ASSERT_EQ(APP_COMMON_FATAL_TEST_CANARY, TEST_MEMORY[i]);
    ASSERT_EVENTS_AppFatalHandler_OcmClear_SIZE(1);
  }
  void Tester :: full_warning(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    for (i = 0; i < (APP_FATAL_RECORDS + 1)/2; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_EVENTS_AppFatalHandler_OcmFull(0, 50);
    for (i = 0; i < (APP_FATAL_RECORDS + 1)/2; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_EVENTS_AppFatalHandler_OcmFull(1, 100);

  }
  void Tester :: fatal_log(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    for (i = 0; i < (APP_FATAL_RECORDS + 1)/2; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_from_storedEvrOut_SIZE((APP_FATAL_RECORDS + 1)/2);
    //Check memory and sent events (via port)
    for (i = 0; i < (APP_FATAL_RECORDS + 1)/2; i++) {
        this->check_downlinked_evr(i);
    }
  }
  void Tester :: fatal_overflow_drop(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    for (i = 0; i < APP_FATAL_RECORDS + 12; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_from_storedEvrOut_SIZE(APP_FATAL_RECORDS);
    //Check memory and sent events (via port)
    for (i = 0; i < APP_FATAL_RECORDS ; i++) {
      this->check_downlinked_evr(i);
    }
  }
  void Tester :: small_region_drop(void)
  {
    NATIVE_INT_TYPE i;
    this->component.m_regionEnd = this->component.m_regionStart + 10;
    this->clear_memory();
    for (i = 0; i < APP_FATAL_RECORDS + 12; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_from_storedEvrOut_SIZE(0);
  }
  void Tester :: memory_not_initialized(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    TEST_MEMORY_STATUS = 0; //Clear the memory status
    for (i = 0; i < APP_FATAL_RECORDS + 12; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    component.preamble();
    ASSERT_from_storedEvrOut_SIZE(0);
    ASSERT_EVENTS_AppFatalHandler_OCM_NOT_INITIALIZED_SIZE(1);
  }
  void Tester :: memory_corrupt(void)
  {
    NATIVE_INT_TYPE i;
    this->clear_memory();
    for (i = 0; i < APP_FATAL_RECORDS; i++) {
      this->sendCmd_APP_FATAL_HDLR_FATAL(0, 0);
      this->component.doDispatch();
    }
    //Corrupt first 2 fatals
    TEST_MEMORY[0] = 2; //Size too low
    TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE] = APP_FATAL_RECORD_MAX_SIZE + 5; //size to big
    component.preamble(); //Send down fatals
    ASSERT_from_storedEvrOut_SIZE(8); //Expect 8 good fatals
    ASSERT_EVENTS_AppFatalHandler_OcmInvalidSize_SIZE(2); //And 2 bad sizes
  }
  void Tester :: ping_test(void)
  {
    this->component.PingRecv_handler(0, PING_KEY);
    ASSERT_from_PingResponse_SIZE(1);
    ASSERT_from_PingResponse(0, PING_KEY);
  }
  // Helper functions
  /**
   * Resets the test memory
   */
  void Tester :: clear_memory() {
      memset(TEST_MEMORY, 0, sizeof(TEST_MEMORY));
      //Set a value to detect buffer overflow
      TEST_MEMORY[APP_FATAL_RECORD_MAX_SIZE*APP_FATAL_RECORDS] = APP_COMMON_FATAL_TEST_CANARY;
      TEST_MEMORY_STATUS = SPHINX_OCM_VALID_FLAG_VALUE;
      this->component.clearSpace();
  }
  void Tester :: check_downlinked_evr(NATIVE_INT_TYPE index) {
    FromPortEntry_storedEvrOut out;
    out = this->fromPortHistory_storedEvrOut->at(index);
    ASSERT_from_storedEvrOut(index, out.id, out.timeTag, out.severity, out.args);
    ASSERT_EQ(out.args.getBuffLength(), 3);

    U8 val0 = 0;
    U8 val1 = 0;
    U8 val2 = 0;
    FW_ASSERT(out.args.deserialize(val0) == Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(APP_FATAL_TEST_BYTE_0, val0);
    FW_ASSERT(out.args.deserialize(val1) == Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(APP_FATAL_TEST_BYTE_1, val1);
    FW_ASSERT(out.args.deserialize(val2) == Fw::FW_SERIALIZE_OK);
    ASSERT_EQ(APP_FATAL_TEST_BYTE_2, val2);
  }

  // ----------------------------------------------------------------------
  // Handlers for typed from ports
  // ----------------------------------------------------------------------
  void Tester ::
    from_storedEvrOut_handler(
        const NATIVE_INT_TYPE portNum,
        FwEventIdType id,
        Fw::Time &timeTag,
        Fw::LogSeverity severity,
        Fw::LogBuffer &args
    )
  {
    this->pushFromPortEntry_storedEvrOut(id, timeTag, severity, args);
  }

  void Tester ::
    from_PingResponse_handler(
        const NATIVE_INT_TYPE portNum,
        U32 key
    )
  {
    this->pushFromPortEntry_PingResponse(key);
  }

  // ----------------------------------------------------------------------
  // Helper methods 
  // ----------------------------------------------------------------------

  void Tester ::
    connectPorts(void) 
  {

    // fatalReceive
    this->connect_to_fatalReceive(
        0,
        this->component.get_fatalReceive_InputPort(0)
    );

    // cmdIn
    this->connect_to_cmdIn(
        0,
        this->component.get_cmdIn_InputPort(0)
    );

    // PingRecv
    this->connect_to_PingRecv(
        0,
        this->component.get_PingRecv_InputPort(0)
    );

    // timeCaller
    this->component.set_timeCaller_OutputPort(
        0, 
        this->get_from_timeCaller(0)
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

    // storedEvrOut
    this->component.set_storedEvrOut_OutputPort(
        0, 
        this->get_from_storedEvrOut(0)
    );

    // PingResponse
    this->component.set_PingResponse_OutputPort(
        0, 
        this->get_from_PingResponse(0)
    );

    // tlmOut
    this->component.set_tlmOut_OutputPort(
        0, 
        this->get_from_tlmOut(0)
    );

    // cmdResponseOut
    this->component.set_cmdResponseOut_OutputPort(
        0, 
        this->get_from_cmdResponseOut(0)
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
