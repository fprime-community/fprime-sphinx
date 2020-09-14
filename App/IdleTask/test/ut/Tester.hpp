// ====================================================================== 
// \title  IdleTask/test/ut/Tester.hpp
// \author mstarch
// \brief  hpp file for IdleTask test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "App/IdleTask/IdleTaskComponentImpl.hpp"

namespace App {
  /**
   * Fake Memory allocation to replace hardware registers
   */
  struct FakeMemory {
      U32 SDRAM[4];
      U32 OCM[4];
      U32 OTHER_MEM[4];
      U32 CORRECT[2];
      U32 BOOT;
      U32 CHECKSUM;
      U32 VALID;
      U32 RESET;
      //Partition flags
      U32 PUTIL;
      U32 PENG;
      U32 PSCI;
      U32 PSEQ;
      //Registers
      U32 AHB_STATUS;
      U32 FAILING;
      U32 FTAHBRAM;
      FakeMemory();
      void nukePartition();
      void nukeChecksum();
  };
  class Tester :
    public IdleTaskGTestBase
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

      //! Test for memory corruption
      //!
      void test_corruption(void);
      //! Test that startup telemeters data
      void test_start_telem(void);
      //! Test that changing value telemeters
      void test_change_telem(void);
      //! Test a bad checksum causes initialization
      //! and test bad flags with a good checksum also initializes
      void test_checksum_init(void);
      //! Test that a hard reset flag causes initialization
      void test_reset_init(void);
      //! Test that a good checksum, and no hard reset
      //! does not initialize
      void test_uninit(void);
      //! Test that errors are detectable and that the addresses
      //! are properly listed for sbes
      void test_detect_sbes_errors(void);
      //! Test that errors are detectable and that the addresses
      //! are properly listed for dbes
      void test_detect_dbes_errors(void);
      //! Test preamble with prmDb load failure
      void test_preamble_load_fail(void);
      //! Test commands
      void test_cmds(void);
      //! Test Key Telemetry
      void test_key_tlm(void);
      //! Helper: create an error in fake memory
      //!< \param bool ocm: in OCM or not
      //!< \param bool sbe: single bit error or not
      //!< \param bool notInSDRAMOrOCM: error in SDRAM/OCM or not
      void create_error(bool ocm, bool sbe, bool notInSDRAMOrOCM=false);
      //! Helper: check OCM is as originally setup
      void check_original_ocm(void);
      //! Helper: check memory is in newly initialized state
      void check_inited(void);
      //! Helper: check that memory has not been touched
      void check_uninited(void);
    private:

      // ----------------------------------------------------------------------
      // Handlers for typed from ports
      // ----------------------------------------------------------------------

      //! Handler for from_prmGet
      //!
      Fw::ParamValid from_prmGet_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwPrmIdType id, /*!< Parameter ID*/
          Fw::ParamBuffer &val /*!< Buffer containing serialized parameter value*/
      );

      //! Handler for from_prmSet
      //!
      void from_prmSet_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwPrmIdType id, /*!< Parameter ID*/
          Fw::ParamBuffer &val /*!< Buffer containing serialized parameter value*/
      );

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
      IdleTaskComponentImpl component;
      //Memory locations!
     FakeMemory m_mem;

  };

} // end namespace App

#endif
