// ====================================================================== 
// \title  IdleTask.hpp
// \author mstarch
// \brief  cpp file for IdleTask test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 
#include "Tester.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include <Utils/Hash/Hash.hpp>
#include <fprime-sphinx-drivers/OCM/ocm.h>
#include <cstring>
#include <Utils/TestUtils.hpp>

#define INSTANCE 0
#define CMD_SEQ 0
#define MAX_HISTORY_SIZE 10

static U8 prm_failure = 0;
static U32 resetCnt = 0;

namespace App {

  U32 good_checksum() {
      U32 checksum = 0;
      U32 data[4];
      std::memset(data, 0, sizeof(data));
      Utils::Hash hash;
      hash.update((char*)data, sizeof(data));
      hash.final(checksum);
      return checksum;
  }
  U32 good_checksum_redo() {
      U32 checksum = 0;
      U32 data[4];
      std::memset(data, 0, sizeof(data));
      data[0] = 1;
      data[2] = 1;
      Utils::Hash hash;
      hash.update((char*)data, sizeof(data));
      hash.final(checksum);
      return checksum;
  }

  FakeMemory :: 
    FakeMemory() :
      BOOT(0),
      CHECKSUM(good_checksum()),
      VALID(0),
      RESET(0),
      PUTIL(0),
      PENG(0),
      PSCI(0),
      PSEQ(0),
      AHB_STATUS(0),
      FAILING(0),
      FTAHBRAM(0)
    {
        SDRAM[0] = 0xDEADBEEF;
        SDRAM[1] = 0xFEEDBEEF;
        SDRAM[2] = 0xDEADBEEF;
        SDRAM[3] = 0xFEEDBEEF;
        OCM[0] = 0xFEEDBEEF;
        OCM[1] = 0xDEADBEEF;
        OCM[2] = 0xFEEDBEEF;
        OCM[3] = 0xDEADBEEF;
        CORRECT[0] = 0xDEADBEEF;
        CORRECT[1] = 0xFEEDBEEF;
    } 
  void FakeMemory ::
    nukePartition()
  {
      PUTIL = 0xDEADBEEF;
      PENG = 0xFEEDDEAD;
      PSCI = 0xDEEDDEED;
      PSEQ = 0xBEEFFEED;
  }
  void FakeMemory ::
    nukeChecksum()
  {
      CHECKSUM = 0xFFEEDDCC;
  }
  // ----------------------------------------------------------------------
  // Construction and destruction 
  // ----------------------------------------------------------------------

  Tester ::
    Tester(void) : 
#if FW_OBJECT_NAMES == 1
      IdleTaskGTestBase("Tester", MAX_HISTORY_SIZE),
      component("IdleTask"),
#else
      IdleTaskGTestBase(MAX_HISTORY_SIZE)
#endif
      m_mem()
  {
    //Setup memory locations
    component.m_sdram_start = &m_mem.SDRAM[0];
    component.m_sdram_end = &m_mem.SDRAM[sizeof(m_mem.SDRAM)/sizeof(m_mem.SDRAM[0])];
    component.m_ocm_boot = &m_mem.BOOT;
    component.m_ocm_start = &m_mem.OCM[0];
    component.m_ocm_end = &m_mem.OCM[sizeof(m_mem.OCM)/sizeof(m_mem.OCM[0])];
    component.m_checksum_addr = &m_mem.CHECKSUM;
    component.m_valid_addr = &m_mem.VALID;
    component.m_reset_reg = &m_mem.RESET;
    component.m_putil_addr = &m_mem.PUTIL;
    component.m_peng_addr = &m_mem.PENG;
    component.m_psci_addr = &m_mem.PSCI;
    component.m_pseq_addr = &m_mem.PSEQ;
    component.m_ahb_status_addr = &m_mem.AHB_STATUS;
    component.m_failing_addr = &m_mem.FAILING;
    component.m_ftahbram_addr = &m_mem.FTAHBRAM;

    //Continue setup
    this->initComponents();
    this->connectPorts();
  }

  Tester ::
    ~Tester(void) 
  {}
  // ----------------------------------------------------------------------
  // Tests 
  // ----------------------------------------------------------------------
  // Tests run:
  //  1. Rewrite memory corruption: DONE
  //  2. Start-up telemeter runs: DONE
  //  3. Telemeter runs on change: DONE
  // OCM tests: UNDONE
  //  4. Initialize OCM: DONE
  //  5. Initialize OCM on checksum: DONE
  //  6. Initialize OCM on hard reset: 
  //  7. No initialize OCM on soft boot, good checksum: DONE
 
  void Tester ::
    test_corruption(void) 
  {
      this->component.cycle();
      for (U32 i = 0; i < sizeof(m_mem.SDRAM)/sizeof(m_mem.SDRAM[0]); i++)
      {
          ASSERT_EQ(m_mem.SDRAM[i], m_mem.CORRECT[i % (sizeof(m_mem.CORRECT)/sizeof(m_mem.CORRECT[0]))]);
      }
      check_original_ocm();
  }
  void Tester ::
    test_start_telem(void)
  {
     this->component.preamble();
     ASSERT_TLM_IdleTask_sbe_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc5_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc5_SIZE(1);
     ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(1);
     ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(1); 
  }
  void Tester ::
    test_change_telem(void)
  {
     //Start should produce 1 telemeter
     test_start_telem();
     //A cycle should not produce output without change
     component.telemeter();
     ASSERT_TLM_IdleTask_sbe_SIZE(1);
     ASSERT_TLM_IdleTask_dbe_SIZE(1);
     ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc5_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc5_SIZE(1);
     //Force a change on two items, those should get addition telemeter
     component.m_current[0] = 23;
     component.m_current[3] = 0xFEEDFEED;
     component.telemeter();
     ASSERT_TLM_IdleTask_sbe_SIZE(2);
     ASSERT_TLM_IdleTask_sbe(1,23);
     ASSERT_TLM_IdleTask_dbe_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
     ASSERT_TLM_IdleTask_sbe_loc2(1,0xFEEDFEED);
     ASSERT_TLM_IdleTask_sbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc5_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc5_SIZE(1);
     //Did the change stick
     component.telemeter();
     ASSERT_TLM_IdleTask_sbe_SIZE(2);
     ASSERT_TLM_IdleTask_dbe_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
     ASSERT_TLM_IdleTask_sbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_sbe_loc5_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc2_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc3_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc4_SIZE(1); 
     ASSERT_TLM_IdleTask_dbe_loc5_SIZE(1);
  }
  void Tester ::
    test_checksum_init(void)
  {
      //Good checksum, bad partitions
      m_mem.nukePartition();
      m_mem.RESET = IDLE_TASK_OCM_SOFT_RESET_VAL;
      ASSERT_NE(0, m_mem.PUTIL);
      ASSERT_NE(0, m_mem.PENG);
      ASSERT_NE(0, m_mem.PSCI);
      ASSERT_NE(0, m_mem.PSEQ);
      ASSERT_EQ(good_checksum(), m_mem.CHECKSUM);
      ASSERT_NE(IDLE_TASK_OCM_HARD_RESET_VAL, m_mem.RESET);
      ASSERT_EQ(0, m_mem.VALID);
      //Try to initialize 
      component.initializeOCM();
      this->component.preamble();
      ASSERT_EVENTS_IdleTask_PartitionFlagChecksumFailure_SIZE(1);
      check_inited();
      //Unclear OCM
      m_mem.OCM[0] = 123;
      m_mem.OCM[2] = 123;
      m_mem.OCM[3] = 123;
      m_mem.OCM[4] = 123;
      //Good partitions, bad checksum
      m_mem.nukeChecksum();
      ASSERT_EQ(0, m_mem.PUTIL);
      ASSERT_EQ(0, m_mem.PENG);
      ASSERT_EQ(0, m_mem.PSCI);
      ASSERT_EQ(0, m_mem.PSEQ);
      ASSERT_NE(good_checksum(), m_mem.CHECKSUM);
      ASSERT_NE(IDLE_TASK_OCM_HARD_RESET_VAL, m_mem.RESET);
      //Try to initialize
      component.initializeOCM();
      check_inited();
      this->component.preamble();
      ASSERT_EVENTS_IdleTask_PartitionFlagChecksumFailure_SIZE(2);
  }
  void Tester ::
    test_reset_init(void)
  {
      m_mem.RESET = IDLE_TASK_OCM_HARD_RESET_VAL;
      //Good flags, good checksum, hard reset true
      ASSERT_EQ(0, m_mem.PUTIL);
      ASSERT_EQ(0, m_mem.PENG);
      ASSERT_EQ(0, m_mem.PSCI);
      ASSERT_EQ(0, m_mem.PSEQ);
      ASSERT_EQ(good_checksum(), m_mem.CHECKSUM);
      ASSERT_EQ(IDLE_TASK_OCM_HARD_RESET_VAL, m_mem.RESET);
      ASSERT_EQ(0, m_mem.VALID);
      //Try initialization
      component.initializeOCM();
      check_inited();
      this->component.preamble();
      ASSERT_EVENTS_IdleTask_HardRebootDetected_SIZE(1);

      this->clearHistory();
      resetCnt = IDLE_TASK_MAX_POR_CNT_VAL; // at boundary, within limit
      this->component.preamble();
      ASSERT_EVENTS_IdleTask_PwrOnResetCnt_WrapAround_SIZE(1);
  }
  void Tester ::
    test_uninit(void)
  {
      //Good flags, good checksum, no hard reset
      m_mem.RESET = IDLE_TASK_OCM_SOFT_RESET_VAL;
      ASSERT_EQ(0, m_mem.PUTIL);
      ASSERT_EQ(0, m_mem.PENG);
      ASSERT_EQ(0, m_mem.PSCI);
      ASSERT_EQ(0, m_mem.PSEQ);
      ASSERT_EQ(good_checksum(), m_mem.CHECKSUM);
      ASSERT_EQ(IDLE_TASK_OCM_SOFT_RESET_VAL, m_mem.RESET);
      ASSERT_EQ(0, m_mem.VALID);
      //Try initialization
      component.initializeOCM();
      check_uninited();
  }
  void Tester ::
    test_detect_sbes_errors() {
      //1 single bit error
      create_error(true, true);
      component.cycle();
      ASSERT_EQ(m_mem.FTAHBRAM >> 13, 0);
      ASSERT_TLM_IdleTask_sbe_SIZE(1);
      ASSERT_TLM_IdleTask_sbe(0, 1);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //2 single bit errors
      create_error(false, true);
      component.cycle();
      ASSERT_TLM_IdleTask_sbe_SIZE(2);
      ASSERT_TLM_IdleTask_sbe(1, 2);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      //Red single bit error (3)
      create_error(false, true);
      component.cycle();
      ASSERT_TLM_IdleTask_sbe_SIZE(3);
      ASSERT_TLM_IdleTask_sbe(2, 3);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      //Blue single bit error (4)
      create_error(true, true);
      component.cycle();
      ASSERT_EQ(m_mem.FTAHBRAM >> 13, 0);
      ASSERT_TLM_IdleTask_sbe_SIZE(4);
      ASSERT_TLM_IdleTask_sbe(3, 4);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //555 - 5th single bit error
      create_error(true, true);
      component.cycle();
      ASSERT_EQ(m_mem.FTAHBRAM >> 13, 0);
      ASSERT_TLM_IdleTask_sbe_SIZE(5);
      ASSERT_TLM_IdleTask_sbe(4, 5);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc5_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc5(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //6 - 6th single bit error
      create_error(false, true);
      component.cycle();
      ASSERT_TLM_IdleTask_sbe_SIZE(6);
      ASSERT_TLM_IdleTask_sbe(5, 6);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc5_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc1(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc5(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //7 - 7th single bit error, outside SDRAM and OCM
      create_error(false, true, true);
      component.cycle();
      ASSERT_TLM_IdleTask_sbe_SIZE(7);
      ASSERT_TLM_IdleTask_sbe(6, 7);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(3);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc5_SIZE(2);
      ASSERT_TLM_IdleTask_sbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc1(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc2(2, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OTHER_MEM[1])));
      ASSERT_TLM_IdleTask_sbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_sbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_sbe_loc5(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_EVENTS_SIZE(1);
      ASSERT_EVENTS_IdleTask_SBE_UncorrectedInMem_SIZE(1);
      ASSERT_EVENTS_IdleTask_SBE_UncorrectedInMem(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OTHER_MEM[1])));
      //Check DBEs are 0 (1 with initial downlink)
      ASSERT_TLM_IdleTask_dbe_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc3_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc4_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc5_SIZE(1);
  }
  void Tester ::
    test_detect_dbes_errors() {
      //1 single bit error
      create_error(true, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(1);
      ASSERT_TLM_IdleTask_dbe(0, 1);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //2 single bit errors
      create_error(false, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(2);
      ASSERT_TLM_IdleTask_dbe(1, 2);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      //Red single bit error (3)
      create_error(false, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(3);
      ASSERT_TLM_IdleTask_dbe(2, 3);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      //Blue single bit error (4)
      create_error(true, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(4);
      ASSERT_TLM_IdleTask_dbe(3, 4);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //555 - 5th single bit error
      create_error(true, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(5);
      ASSERT_TLM_IdleTask_dbe(4, 5);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc5_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc5(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //6 - 6th single bit error
      create_error(false, false);
      component.cycle();
      ASSERT_TLM_IdleTask_dbe_SIZE(6);
      ASSERT_TLM_IdleTask_dbe(5, 6);
      ASSERT_TLM_IdleTask_dbe_loc1_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc2_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc3_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc4_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc5_SIZE(2);
      ASSERT_TLM_IdleTask_dbe_loc1(0, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc1(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc2(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc3(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1])));
      ASSERT_TLM_IdleTask_dbe_loc4(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      ASSERT_TLM_IdleTask_dbe_loc5(1, static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1])));
      //Check SBEs are 0 (1 with initial downlink)
      ASSERT_TLM_IdleTask_sbe_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc1_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc2_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc3_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc4_SIZE(1);
      ASSERT_TLM_IdleTask_sbe_loc5_SIZE(1);
  }

  void Tester ::
    test_preamble_load_fail(void) {
      this->clearHistory();
      // indicate hard boot
      this->component.m_startup_evr = HARD_RESET;

      prm_failure = 1;
      resetCnt = IDLE_TASK_PWR_ON_RESET_CNT_DFLT;
      this->component.preamble();
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(1); 
      ASSERT_TLM_IdleTask_Power_On_Reset_Count(0, resetCnt+1); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(1); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val(0, resetCnt+1); 
      ASSERT_from_prmSet_SIZE(2); // 1 from setting dflt, 1 from updating incremented val
      this->clearHistory();

      prm_failure = 0;
      resetCnt = 5;
      this->component.preamble();
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(1); 
      ASSERT_TLM_IdleTask_Power_On_Reset_Count(0, resetCnt+1); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(1); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val(0, resetCnt+1); 
      ASSERT_from_prmSet_SIZE(1); // 1 from updating incremented val
  }

  void Tester ::
    test_cmds(void) {
      this->clearHistory();

      resetCnt = IDLE_TASK_MAX_POR_CNT_VAL; // at boundary, within limit
      this->sendCmd_IDLE_TASK_SET_PWR_ON_RST_CNT(INSTANCE, CMD_SEQ, resetCnt);
      // Assert command response
      ASSERT_CMD_RESPONSE_SIZE(1);
      ASSERT_CMD_RESPONSE(
          0,
          IdleTaskComponentBase::OPCODE_IDLE_TASK_SET_PWR_ON_RST_CNT,
          CMD_SEQ,
          Fw::COMMAND_OK
      );  
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(1); 
      ASSERT_TLM_IdleTask_Power_On_Reset_Count(0, resetCnt); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(1);
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val(0, resetCnt);

      this->sendCmd_IDLE_TASK_DMP_PWR_ON_RST_CNT(INSTANCE, CMD_SEQ);
      // Assert command response
      ASSERT_CMD_RESPONSE_SIZE(2);
      ASSERT_CMD_RESPONSE(
          1,
          IdleTaskComponentBase::OPCODE_IDLE_TASK_DMP_PWR_ON_RST_CNT,
          CMD_SEQ,
          Fw::COMMAND_OK
      );  
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(2); 
      ASSERT_TLM_IdleTask_Power_On_Reset_Count(1, resetCnt); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(2);
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val(1, resetCnt);

      this->clearHistory();

      resetCnt = IDLE_TASK_MAX_POR_CNT_VAL + 1; // above max limit
      this->sendCmd_IDLE_TASK_SET_PWR_ON_RST_CNT(INSTANCE, CMD_SEQ, resetCnt);
      // Assert command response
      ASSERT_CMD_RESPONSE_SIZE(1);
      ASSERT_CMD_RESPONSE(
          0,
          IdleTaskComponentBase::OPCODE_IDLE_TASK_SET_PWR_ON_RST_CNT,
          CMD_SEQ,
          Fw::COMMAND_VALIDATION_ERROR
      );  
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_Invalid_SIZE(1);
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(0); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(0);

      this->sendCmd_IDLE_TASK_DMP_PWR_ON_RST_CNT(INSTANCE, CMD_SEQ);
      // Assert command response
      ASSERT_CMD_RESPONSE_SIZE(2);
      ASSERT_CMD_RESPONSE(
          1,
          IdleTaskComponentBase::OPCODE_IDLE_TASK_DMP_PWR_ON_RST_CNT,
          CMD_SEQ,
          Fw::COMMAND_OK
      );  
      // Verify reset count is last valid value set
      ASSERT_TLM_IdleTask_Power_On_Reset_Count_SIZE(1); 
      ASSERT_TLM_IdleTask_Power_On_Reset_Count(0, IDLE_TASK_MAX_POR_CNT_VAL); 
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val_SIZE(1);
      ASSERT_EVENTS_IdleTask_PowerOnReset_Val(0, IDLE_TASK_MAX_POR_CNT_VAL);
  }

  // ----------------------------------------------------------------------
  // Helper methods 
  // ----------------------------------------------------------------------

  void Tester ::
    create_error(bool ocm, bool sbe, bool notInSDRAMOrOCM) {
      m_mem.AHB_STATUS = 0x00000100 | (static_cast<U32>(sbe) << 9);
      if (ocm) {
          if (sbe) {
              m_mem.FTAHBRAM = ((m_mem.FTAHBRAM >> 13) + 1) << 13;
          }
          m_mem.FAILING = static_cast<U32>(reinterpret_cast<U64>(&m_mem.OCM[1]));
      } else {
          if (notInSDRAMOrOCM) {
            m_mem.FAILING = static_cast<U32>(reinterpret_cast<U64>(&m_mem.OTHER_MEM[1]));
          } else {
            m_mem.FAILING = static_cast<U32>(reinterpret_cast<U64>(&m_mem.SDRAM[1]));
          }
      }

  }
  void Tester ::
    check_inited(void)
  {
      ASSERT_EQ(0, m_mem.PUTIL);
      ASSERT_EQ(0, m_mem.PENG);
      ASSERT_EQ(0, m_mem.PSCI);
      ASSERT_EQ(0, m_mem.PSEQ);
      for (U32 i = 0; i < sizeof(m_mem.OCM)/sizeof(m_mem.OCM[0]); i++)
      {
          ASSERT_EQ(0, m_mem.OCM[i]);
      }
      ASSERT_EQ(good_checksum(), m_mem.CHECKSUM);
      ASSERT_EQ(SPHINX_OCM_VALID_FLAG_VALUE, m_mem.VALID);
  }
  void Tester ::
    check_uninited(void)
  {
      ASSERT_EQ(0, m_mem.PUTIL);
      ASSERT_EQ(0, m_mem.PENG);
      ASSERT_EQ(0, m_mem.PSCI);
      ASSERT_EQ(0, m_mem.PSEQ);
      check_original_ocm();
      ASSERT_EQ(good_checksum(), m_mem.CHECKSUM);
      ASSERT_EQ(SPHINX_OCM_VALID_FLAG_VALUE, m_mem.VALID);
  }
  void Tester ::
    check_original_ocm(void)
  {
      for (U32 i = 0; i < sizeof(m_mem.OCM)/sizeof(m_mem.OCM[0]); i++)
      {
          ASSERT_EQ(m_mem.CORRECT[(i + 1) % (sizeof(m_mem.CORRECT)/sizeof(m_mem.CORRECT[0]))], m_mem.OCM[i]);
      }
  }

  // ----------------------------------------------------------------------
  // Handlers for typed from ports
  // ----------------------------------------------------------------------

  Fw::ParamValid Tester ::
    from_prmGet_handler(
        const NATIVE_INT_TYPE portNum,
        FwPrmIdType id,
        Fw::ParamBuffer &val
    )
  {
    val.serialize(resetCnt);
    this->pushFromPortEntry_prmGet(id, val);
    if (!prm_failure) {
      return Fw::PARAM_VALID;
    } else {
      return Fw::PARAM_INVALID;
    }
  }

  void Tester ::
    from_prmSet_handler(
        const NATIVE_INT_TYPE portNum,
        FwPrmIdType id,
        Fw::ParamBuffer &val
    )
  {
    this->pushFromPortEntry_prmSet(id, val);
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

      // sphinx_time
      this->component.set_sphinx_time_OutputPort(
          0,
          this->get_from_sphinx_time(0)
      );

      // tlmOut
      this->component.set_tlmOut_OutputPort(
          0,
          this->get_from_tlmOut(0)
      );

      // prmGet
      this->component.set_prmGet_OutputPort(
          0,
          this->get_from_prmGet(0)
      );

      // prmSet
      this->component.set_prmSet_OutputPort(
          0,
          this->get_from_prmSet(0)
      );

      // cmdResponseOut
      this->component.set_cmdResponseOut_OutputPort(
          0,
          this->get_from_cmdResponseOut(0)
      );

      /*// Time
      this->component.set_Time_OutputPort(
          0,
          this->get_from_Time(0)
      );*/

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
          INSTANCE
      );
    }

  } // end namespace App
