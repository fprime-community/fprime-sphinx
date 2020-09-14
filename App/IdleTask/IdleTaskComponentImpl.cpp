// ====================================================================== 
// \title  IdleTaskImpl.cpp
// \author mstarch
// \brief  cpp file for IdleTask component implementation class
//
// \copyright
// Copyright 2009-2018, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ====================================================================== 


#include <App/IdleTask/IdleTaskComponentImpl.hpp>
#include <fprime-sphinx-drivers/OCM/ocm.h>
#include <fprime-sphinx-drivers/Util/SphinxDrvUtil.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/EightyCharString.hpp>
#include <Utils/Hash/Hash.hpp>
#include <fprime-sphinx-drivers/Util/SphinxDrvUtil.hpp>
#include <Fw/Types/Assert.hpp>
#include <App/Include/param.hpp>
#include <Os/InterruptLock.hpp>

namespace App {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction 
  // ----------------------------------------------------------------------

  IdleTaskComponentImpl ::
#if FW_OBJECT_NAMES == 1
    IdleTaskComponentImpl(
        const char *const compName
    ) :
      IdleTaskComponentBase(compName),
#endif
      m_sdram_start((volatile U32* const)IDLE_TASK_SDRAM_START),
      m_sdram_end((volatile U32* const)IDLE_TASK_SDRAM_END),
      m_ocm_boot((volatile U32* const)SPHINX_OCM_BOOT_COUNT_ADDR),
      m_ocm_start((volatile U32* const)SPHINX_OCM_BASE_ADDR),
      m_ocm_end((volatile U32* const)SPHINX_OCM_HIGH_ADDR),
      m_checksum_addr((volatile U32* const)SPHINX_OCM_STARTUP_CHECKSUM_ADDR),
      m_valid_addr((volatile U32* const)SPHINX_OCM_VALID_FLAG_ADDR),
      m_reset_reg((volatile U32* const)IDLE_TASK_OCM_HARD_RESET_ADDR),
      m_putil_addr((volatile U32* const)SPHINX_OCM_UTIL_PART_FLAG_ADDR),
      m_peng_addr((volatile U32* const)SPHINX_OCM_ENG_PART_FLAG_ADDR),
      m_psci_addr((volatile U32* const)SPHINX_OCM_SCI_PART_FLAG_ADDR),
      m_pseq_addr((volatile U32* const)SPHINX_OCM_SEQ_PART_FLAG_ADDR),
      m_ahb_status_addr((volatile U32* const)IDLE_TASK_AHB_STATUS_REGISTER),
      m_failing_addr((volatile U32* const)IDLE_TASK_AHB_FAILING_ERROR),
      m_ftahbram_addr((volatile U32* const)IDLE_TASK_FTAHBRAM_ADDRESS),
      m_ocm_sbe_count(0),
      m_sdram_sbe_count(0),
      m_other_mem_sbe_count(0),
      m_dbe_count(0),
      m_sbe_index(0),
      m_dbe_index(0),
      m_startup_evr(NO_RESET)
  {
      //Initialized telemetered and current values must be different
      for (int i = 0; i < IDLE_TASK_CHANNEL_COUNT; i++)
      {
          m_telemetered[i] = 0xFFFFFFFF;
          m_current[i] = 0x00000000;
      }

      // Initialize rate limiter
      this->m_rateLimiter_sbe_uncorrected.setCounterCycle(RATE_LIMIT_SBE_CORRECTED_CNT);
      this->m_rateLimiter_sbe_uncorrected.setTimeCycle(RATE_LIMIT_SBE_CORRECTED_SECS);
  }

  void IdleTaskComponentImpl ::
    init(
        const NATIVE_INT_TYPE instance
    ) 
  {
    IdleTaskComponentBase::init(instance);
  }
  U32 IdleTaskComponentImpl ::
     partitionChecksum(void)
  {
      U32 checksum = 0;
      U32 data[4];
      Utils::Hash hash;
      FW_ASSERT(m_putil_addr != NULL);
      FW_ASSERT(m_peng_addr != NULL);
      FW_ASSERT(m_psci_addr != NULL);
      FW_ASSERT(m_pseq_addr != NULL);
      data[0] = *m_putil_addr;
      data[1] = *m_peng_addr;
      data[2] = *m_psci_addr;
      data[3] = *m_pseq_addr;
      hash.update((char*)data, sizeof(data));
      hash.final(checksum);  
      return checksum;
  }
  void IdleTaskComponentImpl ::
    initializeOCM(void)
  {
      FW_ASSERT(m_reset_reg != NULL);
      FW_ASSERT(m_checksum_addr != NULL);
      FW_ASSERT(m_ocm_boot != NULL);
      FW_ASSERT(m_ocm_start != NULL);
      FW_ASSERT(m_ocm_end != NULL);
      FW_ASSERT(m_putil_addr != NULL);
      FW_ASSERT(m_peng_addr != NULL);
      FW_ASSERT(m_psci_addr != NULL);
      FW_ASSERT(m_pseq_addr != NULL);
      if ((*m_reset_reg != IDLE_TASK_OCM_SOFT_RESET_VAL) ||
          (*m_checksum_addr != partitionChecksum()))
      {
          //Send an EVR to explain what is happening
          if (*m_reset_reg != IDLE_TASK_OCM_SOFT_RESET_VAL) {
              m_startup_evr = HARD_RESET;
          } else {
              m_startup_evr = SOFT_RESET;
          }
          //Prep OCM for fatal storage
          U32 loopVar = 0;
          FW_ASSERT(m_ocm_start < m_ocm_end);
          volatile U32* clearable = m_ocm_start;
          while (clearable < m_ocm_end && loopVar < (SPHINX_OCM_HIGH_ADDR - SPHINX_OCM_BASE_ADDR))
          {
              *(clearable) = 0;
              clearable += 1;
              loopVar += 1;
          }
          //Per Charles: reset to 0xDEADBEEF **Note:** must set hardboot after clear for EDAC purposes
          *m_reset_reg = IDLE_TASK_OCM_SOFT_RESET_VAL;
          *m_ocm_boot = 0;
          *m_putil_addr = 0;
          *m_peng_addr = 0;
          *m_psci_addr = 0;
          *m_pseq_addr = 0;
          updateChecksum();
      }
      *m_valid_addr = SPHINX_OCM_VALID_FLAG_VALUE;
  }
  void IdleTaskComponentImpl ::
    preamble(void) {
      bool load_success;

      // Load last stored value of power on reset count from prmDb
      load_success = this->loadPwrOnResetCnt();
      if (!load_success) {
        // If failed to load pwrOnResetCnt from prmDb then set
        // value to default
        this->m_pwrOnResetCnt.setPwrOnResetCnt(IDLE_TASK_PWR_ON_RESET_CNT_DFLT);

        // update in-memory PrmDb parameter
        Fw::ParamBuffer pbuf;
        Fw::SerializeStatus status;
        status = pbuf.serialize(this->m_pwrOnResetCnt);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK);

        this->prmSet_out(0, IDLE_TASK_PWR_ON_RESET_CNT_PARAM, pbuf);
      }

      //TODO: set sbe and dbe error counts from OCM from boot-startup memory check
      //m_sbe_sdram_count = <sdram startup count>
      //m_dbe_count = <sdram + ocm startup count>
      //TODO: do we need to copy some DC bias from the ocm count or is it also captured in FTHAM reg?
      readEccs();
      telemeter();
      if (m_startup_evr == HARD_RESET) {
          // Increment pwrOnResetCnt upon hard reset
          this->updatePwrOnResetCnt();
          this->log_ACTIVITY_HI_IdleTask_HardRebootDetected();
      } else if (m_startup_evr == SOFT_RESET) {
          this->log_WARNING_HI_IdleTask_PartitionFlagChecksumFailure();
      }

      // telemeter latest power on reset count value
      // after it has been loaded from prmDb and updated with
      // new incremented value if applicable for this boot.
      U32 bootVal = this->m_pwrOnResetCnt.getPwrOnResetCnt();
      this->tlmWrite_IdleTask_Power_On_Reset_Count(bootVal);
      this->log_ACTIVITY_HI_IdleTask_PowerOnReset_Val(bootVal);
  }

  bool IdleTaskComponentImpl ::
    loadPwrOnResetCnt(void)
  {
    Fw::ParamBuffer pbuf;
    Fw::ParamValid status = this->prmGet_out(0, IDLE_TASK_PWR_ON_RESET_CNT_PARAM, pbuf);
    if (status != Fw::PARAM_VALID) {
      // NB: PrmDb will already send evr
      return false;
    }
    Fw::SerializeStatus serializeStatus = pbuf.deserialize(this->m_pwrOnResetCnt);
    if (serializeStatus != Fw::FW_SERIALIZE_OK) {
      this->log_WARNING_LO_IdleTask_FailedToDeserializePwrOnResetCnt(serializeStatus);
      return false;
    }

    return true;
  }

  void IdleTaskComponentImpl ::
    updatePwrOnResetCnt(void)
  {
    U32 val;
 
    // Current value of pwrOnReset has already been loaded
    // so increment it's value by 1 and set it in prmDb
    val = this->m_pwrOnResetCnt.getPwrOnResetCnt();
    if (val >= IDLE_TASK_MAX_POR_CNT_VAL) {
      val = val % IDLE_TASK_MAX_POR_CNT_VAL; // wrap-around after reaching max allowable POR count val
      this->log_WARNING_LO_IdleTask_PwrOnResetCnt_WrapAround();
    }
    val += 1;
    
    this->m_pwrOnResetCnt.setPwrOnResetCnt(val);

    // update in-memory PrmDb parameter
    Fw::ParamBuffer pbuf;
    Fw::SerializeStatus status;
    status = pbuf.serialize(this->m_pwrOnResetCnt);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK);

    this->prmSet_out(0, IDLE_TASK_PWR_ON_RESET_CNT_PARAM, pbuf);      
  }

  void IdleTaskComponentImpl ::
    updateChecksum(void)
  {
      FW_ASSERT(m_checksum_addr != NULL);
      *m_checksum_addr = partitionChecksum();
  }

  void IdleTaskComponentImpl ::
    start(const NATIVE_INT_TYPE identifier, const I32 priority)
  {
      Fw::EightyCharString name("IdleTask-MemoryRefresher");
      Os::Task::TaskStatus stat = m_idle_task.start(name, identifier, priority, IDLE_TASK_STACK, IdleTaskComponentImpl::idleTask, (void*)this);
      FW_ASSERT(Os::Task::TASK_OK == stat,static_cast<NATIVE_INT_TYPE>(stat));
  }

  void IdleTaskComponentImpl ::
    readEccs(void)
  {
    //Update OCM sbe error count from counting register, add it to existing count, and then clear their count
    m_ocm_sbe_count = m_ocm_sbe_count + ((*m_ftahbram_addr & IDLE_TASK_FTAHBRAM_MASK) >> IDLE_TASK_FTAHBRAM_SHIFT);
    *m_ftahbram_addr = *m_ftahbram_addr & ~IDLE_TASK_FTAHBRAM_MASK;
    //SDRAM SBEs have no HW support, but must be added to OCM SBEs which have hardware support
    m_current[IDLE_TASK_SBE_COUNT_INDEX] = m_sdram_sbe_count + m_ocm_sbe_count + m_other_mem_sbe_count;
    m_current[IDLE_TASK_DBE_COUNT_INDEX] = m_dbe_count;
  }

  void IdleTaskComponentImpl ::
    telemeter(void)
  {
    if ( m_telemetered[0] != m_current[0])
    {
        tlmWrite_IdleTask_sbe(m_current[0]);
        m_telemetered[0] = m_current[0];
    }

    if ( m_telemetered[1] != m_current[1])
    {
        tlmWrite_IdleTask_dbe(m_current[1]);
        m_telemetered[1] = m_current[1];
    }

    if ( m_telemetered[2] != m_current[2])
    {
        tlmWrite_IdleTask_sbe_loc1(m_current[2]);
        m_telemetered[2] = m_current[2];
    }

    if ( m_telemetered[3] != m_current[3])
    {
        tlmWrite_IdleTask_sbe_loc2(m_current[3]);
        m_telemetered[3] = m_current[3];
    }

    if ( m_telemetered[4] != m_current[4])
    {
        tlmWrite_IdleTask_sbe_loc3(m_current[4]);
        m_telemetered[4] = m_current[4];
    }

    if ( m_telemetered[5] != m_current[5])
    {
        tlmWrite_IdleTask_sbe_loc4(m_current[5]);
        m_telemetered[5] = m_current[5];
    }

    if ( m_telemetered[6] != m_current[6])
    {
        tlmWrite_IdleTask_sbe_loc5(m_current[6]);
        m_telemetered[6] = m_current[6];
    }

    if ( m_telemetered[7] != m_current[7])
    {
        tlmWrite_IdleTask_dbe_loc1(m_current[7]);
        m_telemetered[7] = m_current[7];
    }

    if ( m_telemetered[8] != m_current[8])
    {
        tlmWrite_IdleTask_dbe_loc2(m_current[8]);
        m_telemetered[8] = m_current[8];
    }

    if ( m_telemetered[9] != m_current[9])
    {
        tlmWrite_IdleTask_dbe_loc3(m_current[9]);
        m_telemetered[9] = m_current[9];
    }

    if ( m_telemetered[10] != m_current[10])
    {
        tlmWrite_IdleTask_dbe_loc4(m_current[10]);
        m_telemetered[10] = m_current[10];
    }

    if ( m_telemetered[11] != m_current[11])
    {
        tlmWrite_IdleTask_dbe_loc5(m_current[11]);
        m_telemetered[11] = m_current[11];
    }
  }

  void IdleTaskComponentImpl ::
    idleTask(void* in_self)
  {
      FW_ASSERT(in_self != NULL);
      IdleTaskComponentImpl* self = (IdleTaskComponentImpl*) in_self;
      while (1)
      {
          self->cycle();
      }
  }
  void IdleTaskComponentImpl ::
    cycle(void)
  {
      FW_ASSERT(m_sdram_end >= m_sdram_start);
      volatile U32* end = m_sdram_end;
      for (volatile U32* start = m_sdram_start; start < end; start++)
      {
          (void) rewrite(start);
      }
      FW_ASSERT(m_ocm_end >= m_ocm_start);
      end = m_ocm_end;
      for (volatile U32* start = m_ocm_start; start < end; start++)
      {
          (void) rewrite(start);
      }
      readEccs();
      telemeter();
  }

  void IdleTaskComponentImpl ::
    flush_loc(volatile U32* location)
  {
      FW_ASSERT(location != NULL);
#ifdef TGT_OS_TYPE_VXWORKS
      U32 addr = reinterpret_cast<U32>(location);
    __asm__ volatile ("flush %0 "
        :
        : "r"( addr )
    );
#endif
  }

  void IdleTaskComponentImpl ::
    write_back_to_mem(volatile U32* location)
  {
      FW_ASSERT(location != NULL);
      U32 addr=Drv::ptrToU32((U32*)location);

      Os::InterruptLock iLock;

      // If address is within SDRAM or OCM address range
      // then write back to it
      if ((addr >= static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_sdram_start)) &&
          addr < static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_sdram_end))) 
          ||
          (addr >= static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_ocm_start)) &&
          addr < static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_ocm_end)))) {

        // lock interrupts
        iLock.lock();
        // read value
        volatile U32 value = Drv::readReg(addr);
        // write back value
        Drv::writeReg(addr, value);
        // unlock interrupts
        iLock.unLock();
      }
      else {
        // Else increment other mem SBE count and warn that SBE
        // is left uncorrected in memory as failing address was
        // outside SDRAM and OCM
        m_other_mem_sbe_count++;
        if(this->m_rateLimiter_sbe_uncorrected.trigger(this->getTime())) {
          this->log_WARNING_HI_IdleTask_SBE_UncorrectedInMem(addr);
        }
      }    
  }

  void IdleTaskComponentImpl ::
    count_error(volatile U32* location)
  {
      FW_ASSERT(location != NULL);

      //Read and count errors (software emulated error counter)
      FW_ASSERT(m_ahb_status_addr != NULL);
      FW_ASSERT(m_failing_addr != NULL);
      U32 value = *m_ahb_status_addr;
      U32 address = *m_failing_addr;
      if ((value & IDLE_TASK_AHB_STATUS_NEW_ERROR_MASK) != 0)
      {
          //Error counted; clear
          *m_ahb_status_addr = value & ~IDLE_TASK_AHB_STATUS_NEW_ERROR_MASK;
          //Single bit error in SDRAM, add value to on chip count
          if ((value & IDLE_TASK_AHB_STATUS_SBE_MASK) != 0) {
              FW_ASSERT(m_sbe_index + IDLE_TASK_SBE_START_INDEX < IDLE_TASK_DBE_START_INDEX);
              FW_ASSERT(m_sbe_index + IDLE_TASK_SBE_START_INDEX >= IDLE_TASK_SBE_START_INDEX);
              m_current[m_sbe_index + IDLE_TASK_SBE_START_INDEX] = (U32)(address);
              m_sbe_index = (m_sbe_index + 1) % IDLE_TASK_STORED_ADDRESS_COUNT;
              if (address >= static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_sdram_start)) &&
                  address < static_cast<U32>(reinterpret_cast<POINTER_CAST>(m_sdram_end))) {
                  m_sdram_sbe_count += 1;
              }
              write_back_to_mem(location);
              write_back_to_mem(reinterpret_cast<U32*>(address));
          }
          else {
              FW_ASSERT(m_dbe_index + IDLE_TASK_DBE_START_INDEX < IDLE_TASK_CHANNEL_COUNT);
              FW_ASSERT(m_dbe_index + IDLE_TASK_DBE_START_INDEX >= IDLE_TASK_DBE_START_INDEX);
              m_current[m_dbe_index + IDLE_TASK_DBE_START_INDEX] = (U32)(address);
              m_dbe_index = (m_dbe_index + 1) % IDLE_TASK_STORED_ADDRESS_COUNT;
              m_dbe_count += 1;
          }
      }
  }

  U32 IdleTaskComponentImpl ::
    rewrite(volatile U32* location)
  {
      FW_ASSERT(location != NULL);

      count_error(location);
      flush_loc(location);
      volatile U32 value = *location;
      count_error(location);
      return value;
  }

  U32 IdleTaskComponentImpl ::
    get_power_on_reset_count(void)
  {
    return this->m_pwrOnResetCnt.getPwrOnResetCnt();
  }

  IdleTaskComponentImpl ::
    ~IdleTaskComponentImpl(void)
  {

  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void IdleTaskComponentImpl ::
    IDLE_TASK_SET_PWR_ON_RST_CNT_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq,
        U32 value
    )
  {
    Fw::ParamBuffer pbuf;
    Fw::SerializeStatus status;

    if (value > IDLE_TASK_MAX_POR_CNT_VAL) {
      this->log_WARNING_HI_IdleTask_PowerOnReset_Val_Invalid(value, IDLE_TASK_MAX_POR_CNT_VAL);
      this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_VALIDATION_ERROR);   
      return;
    }

    this->m_pwrOnResetCnt.setPwrOnResetCnt(value);

    this->tlmWrite_IdleTask_Power_On_Reset_Count(value);
    this->log_ACTIVITY_HI_IdleTask_PowerOnReset_Val(value);

    // update in-memory PrmDb parameter
    status = pbuf.serialize(this->m_pwrOnResetCnt);
    FW_ASSERT(status == Fw::FW_SERIALIZE_OK);

    this->prmSet_out(0, IDLE_TASK_PWR_ON_RESET_CNT_PARAM, pbuf);      

    this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);   
  }

  void IdleTaskComponentImpl ::
    IDLE_TASK_DMP_PWR_ON_RST_CNT_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    U32 bootVal = this->m_pwrOnResetCnt.getPwrOnResetCnt();
    this->tlmWrite_IdleTask_Power_On_Reset_Count(bootVal);
    this->log_ACTIVITY_HI_IdleTask_PowerOnReset_Val(bootVal);
    this->cmdResponse_out(opCode,cmdSeq,Fw::COMMAND_OK);   
  }

} // end namespace App
