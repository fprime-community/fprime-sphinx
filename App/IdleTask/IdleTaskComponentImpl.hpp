// ====================================================================== 
// \title  IdleTaskImpl.hpp
// \author mstarch
// \brief  hpp file for IdleTask component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 

#ifndef IdleTask_HPP
#define IdleTask_HPP

#include "App/IdleTask/IdleTaskComponentAc.hpp"
#include "App/IdleTask/IdleTaskPwrOnResetCntSerializableAc.hpp"
#include "Utils/RateLimiter.hpp"

#define IDLE_TASK_STACK (10*1024)
#define IDLE_TASK_SDRAM_START 0x40003000
#define IDLE_TASK_SDRAM_END 0x4ff00000

#define IDLE_TASK_MAX_RETRY 100
#define IDLE_TASK_CHANNEL_COUNT 12

#define IDLE_TASK_OCM_HARD_RESET_ADDR 0x200D0000
#define IDLE_TASK_OCM_HARD_RESET_VAL 0xCAFEBABE
#define IDLE_TASK_OCM_SOFT_RESET_VAL 0xDEADBEEF

#define IDLE_TASK_AHB_STATUS_REGISTER 0x80000F00
#define IDLE_TASK_AHB_STATUS_NEW_ERROR_MASK 0x00000100
#define IDLE_TASK_AHB_STATUS_SBE_MASK 0x00000200
#define IDLE_TASK_AHB_FAILING_ERROR 0x80000F04

#define IDLE_TASK_FTAHBRAM_ADDRESS 0x80100000
#define IDLE_TASK_FTAHBRAM_MASK 0x001FE000
#define IDLE_TASK_FTAHBRAM_SHIFT 13

//Indicies
#define IDLE_TASK_STORED_ADDRESS_COUNT 5
#define IDLE_TASK_SBE_COUNT_INDEX 0
#define IDLE_TASK_DBE_COUNT_INDEX 1
#define IDLE_TASK_SBE_START_INDEX 2
#define IDLE_TASK_DBE_START_INDEX (IDLE_TASK_STORED_ADDRESS_COUNT + IDLE_TASK_SBE_START_INDEX)

// Default value for pwrOnResetCnt parameter
// which gets incremented by 1 and stored to
// prmDB at initialization if HARD_RESET is detected 
#define IDLE_TASK_PWR_ON_RESET_CNT_DFLT 0

// Max allowable value for power on reset count
#define IDLE_TASK_MAX_POR_CNT_VAL 99999

// Rate limiter configs
#define RATE_LIMIT_SBE_CORRECTED_CNT 10
#define RATE_LIMIT_SBE_CORRECTED_SECS 10

namespace App {
  enum DownlinkEVRType {
      NO_RESET,
      HARD_RESET,
      SOFT_RESET
  };

  class IdleTaskComponentImpl :
    public IdleTaskComponentBase
  {
    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object IdleTask
      //!
      IdleTaskComponentImpl(
#if FW_OBJECT_NAMES == 1
          const char *const compName /*!< The component name*/
#endif
      );

      //! Initialize object IdleTask
      //!
      void init(
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );
      //! Preamble to run after connections
      void preamble(void);
      //! Calculate checksum on partition flags, assigning it to the OCM
      void updateChecksum(void);
      //! Calculate checksum on partition flags
      U32 partitionChecksum(void);
      //! Initialize the OCM memory, if needed
      void initializeOCM(void);
      //! Get Power On Reset Count
      U32 get_power_on_reset_count(void);
      //! Read ecc statistics from the RAM
      void readEccs();
      //! Telemeter down values of dbes and sbes
      void telemeter(void);
      //! Load pwrOnResetCnt value from prmDB
      bool loadPwrOnResetCnt(void);
      //! Increment and update pwrOnResetCnt value in prmDB
      void updatePwrOnResetCnt(void);
      //! Start the idle task with given identifier and priority
      void start(
          const NATIVE_INT_TYPE identifier, /*!< Identifier of the task*/
          const I32 priority /*!< Priority of the idle task*/
      );
      //! Idle task function
      static void idleTask(
          void* in_self /*!< "this" pointer renaming*/
      );
      //! Cycle once
      void cycle(void);
      //! Count an error, if it exists from GR712 registers
      void count_error(volatile U32* location);
      //! Flush a memory location using flush instruction
      void flush_loc(volatile U32* location);
      //! Read and write back value to a memory location
      void write_back_to_mem(volatile U32* location);

      //! Rewrite a single address
      U32 rewrite(
          volatile U32* location /*!< location to read and write again*/
      );

      //! Destroy object IdleTask
      //!
      ~IdleTaskComponentImpl(void);

    PRIVATE:

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      //! Implementation for IDLE_TASK_SET_PWR_ON_RST_CNT command handler
      //! Set a new power on reset count value. NOTE: PRM_SAVE_FILE command must be sent after setting new value for it to be stored in non-volatile prmDb file.
      void IDLE_TASK_SET_PWR_ON_RST_CNT_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq, /*!< The command sequence number*/
          U32 value /*!< Power on reset count value to be set*/
      );

      //! Implementation for IDLE_TASK_DMP_PWR_ON_RST_CNT command handler
      //! Dump currently active power on reset (POR) count value, i.e. hard boot count.
      void IDLE_TASK_DMP_PWR_ON_RST_CNT_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

    PRIVATE:
      Os::Task m_idle_task;
      //Memory locations, kept as members to enable unit checking
      #ifdef TGT_OS_TYPE_VXWORKS
          volatile U32* const m_sdram_start;
          volatile U32* const m_sdram_end;
          volatile U32* const m_ocm_boot;
          volatile U32* const m_ocm_start;
          volatile U32* const m_ocm_end;
          volatile U32* const m_checksum_addr;
          volatile U32* const m_valid_addr;
          volatile U32* const m_reset_reg;
          //Partition flags
          volatile U32* const m_putil_addr;
          volatile U32* const m_peng_addr;
          volatile U32* const m_psci_addr;
          volatile U32* const m_pseq_addr;
          volatile U32* const m_ahb_status_addr;
          volatile U32* volatile const m_failing_addr;
          volatile U32* const m_ftahbram_addr;
      #else
          volatile U32* m_sdram_start;
          volatile U32* m_sdram_end;
          volatile U32* m_ocm_boot;
          volatile U32* m_ocm_start;
          volatile U32* m_ocm_end;
          volatile U32* m_checksum_addr;
          volatile U32* m_valid_addr;
          volatile U32* m_reset_reg;
          //Partition flags
          volatile U32* m_putil_addr;
          volatile U32* m_peng_addr;
          volatile U32* m_psci_addr;
          volatile U32* m_pseq_addr;
          volatile U32* m_ahb_status_addr;
          volatile U32* m_failing_addr;
          volatile U32* m_ftahbram_addr;
      #endif
      //Counts for different values, and indices
      U32 m_ocm_sbe_count;
      U32 m_sdram_sbe_count;
      U32 m_other_mem_sbe_count;
      U32 m_dbe_count;
      U32 m_sbe_index;
      U32 m_dbe_index;
      //Previous values
      U32 m_telemetered[IDLE_TASK_CHANNEL_COUNT];
      U32 m_current[IDLE_TASK_CHANNEL_COUNT];
      DownlinkEVRType m_startup_evr;

      Utils::RateLimiter m_rateLimiter_sbe_uncorrected;

      // reset count parameter
      IdleTaskPwrOnResetCnt m_pwrOnResetCnt;
      // Key Tlm
  };

} // end namespace App

#endif
