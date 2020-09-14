#include <Components.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Task.hpp>
#include <Os/InterruptLock.hpp>
#include <Os/TaskLock.hpp>
#include <Fw/Logger/Logger.hpp>
#include <Os/Log.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include "FPGAGPIODefATB.hpp"
#include "GPIODefATB.hpp"

#if defined TGT_OS_TYPE_LINUX || TGT_OS_TYPE_DARWIN
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#include <fprime-sphinx-drivers/Util/SphinxDrvUtil.hpp>
#include "GPIODefATB.hpp"
#include "FPGAGPIODefATB.hpp"

// List of context IDs
enum {
    DOWNLINK_PACKET_SIZE = 500,
    DOWNLINK_BUFFER_STORE_SIZE = 2500,
    DOWNLINK_BUFFER_QUEUE_SIZE = 5,
    UPLINK_BUFFER_STORE_SIZE = 3000,
    UPLINK_BUFFER_QUEUE_SIZE = 30
};

Os::Log osLogger;


// Registry
#if FW_OBJECT_REGISTRATION == 1
static Fw::SimpleObjRegistry simpleReg;
#endif

// Component instance pointers
static NATIVE_INT_TYPE rgDivs[Svc::RateGroupDriverImpl::DIVIDER_SIZE] = {1,2,10,100};
Svc::RateGroupDriverImpl rateGroupDriverComp("RGDvr",rgDivs,FW_NUM_ARRAY_ELEMENTS(rgDivs));

static NATIVE_UINT_TYPE rg1Context[] = {0,0,0,0,0,0,0,0,0,0};
Svc::ActiveRateGroupImpl rateGroup1Comp("RG1",rg1Context,FW_NUM_ARRAY_ELEMENTS(rg1Context));

static NATIVE_UINT_TYPE rg2Context[] = {0,0,0,0,0,0,0,0,0,0};
Svc::ActiveRateGroupImpl rateGroup2Comp("RG2",rg2Context,FW_NUM_ARRAY_ELEMENTS(rg2Context));

static NATIVE_UINT_TYPE rg3Context[] = {0,0,0,0,0,0,0,0,0,0};
Svc::ActiveRateGroupImpl rateGroup3Comp("RG3",rg3Context,FW_NUM_ARRAY_ELEMENTS(rg3Context));

static NATIVE_UINT_TYPE rg4Context[] = {0,0,0,0,0,0,0,0,0,0};
Svc::ActiveRateGroupImpl rateGroup4Comp("RG4",rg4Context,FW_NUM_ARRAY_ELEMENTS(rg4Context));

// Command Components
Svc::GroundInterfaceComponentImpl groundIf("GNDIF");

// Driver Component
Drv::FPGADriverComponentImpl fpgaDriver("fpgaDriver");

Drv::FpgaGpioDriverComponentImpl fpgaGpioDriver("fpgaGpioDriver", FPGA_GPIO_TOTAL_AVAIL_PINS, FPGA_GPIO_PINS_ENABLE, FPGA_GPIO_LENGTH);

Drv::GPIODriverComponentImpl gpioDriver("gpioDriver");

Drv::GPIOInterruptRouter gpioIntRouter;

// Apperence Implementation Components

#if FW_ENABLE_TEXT_LOGGING
Svc::ConsoleTextLoggerImpl textLogger("TLOG");
#endif

Svc::ActiveLoggerImpl eventLogger("ELOG");

Drv::SphinxTimeComponentImpl sphinxTime("sphinxTime");

Svc::TlmChanImpl chanTlm("TLM");

Svc::CommandDispatcherImpl cmdDisp("CMDDISP");

Fw::MallocAllocator seqMallocator;
Svc::CmdSequencerComponentImpl cmdSeq("CMDSEQ");

Svc::PrmDbImpl prmDb("PRM","PrmDb.dat");

App::PingReceiverComponentImpl pingRcvr("PngRecv");

Drv::UartDriverComponentImpl uartDriver("uartDriver", "/tyCo/0", 115200);

Svc::FileUplink fileUplink ("fileUplink");

Svc::FileDownlink fileDownlink ("fileDownlink", DOWNLINK_PACKET_SIZE);

Svc::BufferManager fileDownlinkBufferManager("fileDownlinkBufferManager", DOWNLINK_BUFFER_STORE_SIZE, DOWNLINK_BUFFER_QUEUE_SIZE);

Svc::BufferManager fileUplinkBufferManager("fileUplinkBufferManager", UPLINK_BUFFER_STORE_SIZE, UPLINK_BUFFER_QUEUE_SIZE);

Svc::HealthImpl health("health");

Drv::ADCComponentImpl adc("adc", 1, 0, 0, 6);

App::FatalHandlerComponentImpl fatalHandler("fatalHandler");

App::DriverTestComponentImpl driverTest("driverTest");

App::IdleTaskComponentImpl idleTask("idleTask");

Drv::TrapRegistry trap_registrar;

Drv::SPIDriverGenericComponentImpl spiDriverGeneric
#if FW_OBJECT_NAMES == 1
                    ("SPI_DRIVER_GENERIC")
#endif
;

Drv::FPGASPIDriverComponentImpl fpgaSpiDriver
#if FW_OBJECT_NAMES == 1
                    ("FPGA_SPI_DRIVER",
                      FPGA_SPI_1_NUMBER,
                      FPGA_SPI_WIDTH_16
)
#endif
;

Drv::SphinxUartOnboardDriverComponentImpl hw_uart("HW_UART");
Drv::SphinxUartOnboardDriverComponentImpl fw_uart("FW_UART");

App::FSWInfoComponentImpl fswInfo("fswInfo");

bool constructApp(bool dump) {

#if FW_PORT_TRACING
    Fw::PortBase::setTrace(false);
#endif    

    // Data access exception is a double bit error
    trap_registrar.register_trap_handle(0x09, fatalHandler);
    // Instruction access exception is a double bit error in instruction memory
    trap_registrar.register_trap_handle(0x01, fatalHandler);
    // Floating point exceptions are a floating point unit exception
    trap_registrar.register_trap_handle(0x08, fatalHandler); // FPU general exception
    trap_registrar.register_trap_handle(0x2A, fatalHandler); // Divide by zero
    trap_registrar.register_trap_handle(0x04, fatalHandler); //FPU disabled

    // initialize OCM
    idleTask.initializeOCM();

    // Initialize rate group driver
    rateGroupDriverComp.init();

    // Initialize the rate groups
    rateGroup1Comp.init(10,0);
    
    rateGroup2Comp.init(10,1);
    
    rateGroup3Comp.init(10,2);

    rateGroup4Comp.init(10,3);

    // Initialize FPGA driver
    fpgaDriver.init(10);

#if FW_ENABLE_TEXT_LOGGING
    textLogger.init();
#endif

    eventLogger.init(2000,0);
    
    sphinxTime.init(0);

    chanTlm.init(2000,0);

    cmdDisp.init(50,0);

    cmdSeq.init(10,0);
    cmdSeq.allocateBuffer(0,seqMallocator,5*1024);

    prmDb.init(10,0);

    groundIf.init(0);
    uartDriver.init(0);

    adc.init(20);

    fileUplink.init(30, 0);
    fileDownlink.init(30, 0);
    fileUplinkBufferManager.init(0);
    fileDownlinkBufferManager.init(1);
    fatalHandler.init(20, 0);
    health.init(25,0);
    pingRcvr.init(10);

    fswInfo.init(20, 0);
    driverTest.init(20, 0);
    idleTask.init(0);

    spiDriverGeneric.init(0);
    fpgaSpiDriver.init(0);
    gpioIntRouter.init();
    gpioDriver.init(0);
    gpioDriver.init_comp(
        GPIO_PIN_NUMS, GPIO_PIN_LENGTH,
        GPIO_PIN_DIR,  GPIO_PIN_LENGTH,
        GPIO_PIN_INT,  GPIO_PIN_LENGTH,
        GPIO_PIN_POL,  GPIO_PIN_LENGTH,
        GPIO_PIN_EDGE, GPIO_PIN_LENGTH
        ,&gpioIntRouter
    );
    fpgaGpioDriver.init(0);
    hw_uart.init(0);
    hw_uart.init_comp(5, 115200, 40);
    fw_uart.init(0);
    fw_uart.init_comp(7, 115200, 40);

    // Connect rate groups to rate group driver
    constructAppArchitecture();

    // dump topology if requested
    if (dump) {
#if FW_OBJECT_REGISTRATION == 1
        simpleReg.dump();
#endif
        return true;
    }

    /* Register commands */
    cmdSeq.regCommands();
    cmdDisp.regCommands();
    eventLogger.regCommands();
    prmDb.regCommands();
    fileDownlink.regCommands();
    health.regCommands();
    pingRcvr.regCommands();
    adc.regCommands();
    fswInfo.regCommands();
    fatalHandler.regCommands();
    driverTest.regCommands();
    idleTask.regCommands();

    // read parameters
    prmDb.readParamFile();

    // set health ping entries

    Svc::HealthImpl::PingEntry pingEntries[] = {
        {3,5,rateGroup1Comp.getObjName()}, // 0
        {3,5,rateGroup2Comp.getObjName()}, // 1
        {3,5,rateGroup3Comp.getObjName()}, // 2
        {3,5,rateGroup4Comp.getObjName()}, // 3
        {3,5,cmdDisp.getObjName()}, // 4
        {3,5,eventLogger.getObjName()}, // 5
        {3,5,cmdSeq.getObjName()}, // 6
        {3,5,chanTlm.getObjName()}, // 7
        {3,5,prmDb.getObjName()}, // 8
        {3,5,fileUplink.getObjName()}, // 9
        {3,5,fileDownlink.getObjName()}, // 10
        {3,5,pingRcvr.getObjName()}, // 11
        {3,5,adc.getObjName()}, // 12
        {3,5,fswInfo.getObjName()}, // 13
        {3,5,fatalHandler.getObjName()}, // 14
        {3,5,driverTest.getObjName()}, // 15
    };

    // register ping table
    health.setPingEntries(pingEntries,FW_NUM_ARRAY_ELEMENTS(pingEntries),0x123);

    // Active component startup
    // start rate groups
    rateGroup1Comp.start(0, 120,10 * 1024);
    rateGroup2Comp.start(0, 119,10 * 1024);
    rateGroup3Comp.start(0, 118,40 * 1024);
    rateGroup4Comp.start(0, 117,40 * 1024);
    // start FatalHandler
    fatalHandler.start(0, 110, 10 * 1024);
    // start dispatcher
    cmdDisp.start(0,101,40*1024);
    // start sequencer
    cmdSeq.start(0,100,10*1024);
    // start file downlink and uplink
    fileDownlink.start(0, 100, 10*1024);
    fileUplink.start(0, 100, 10*1024);
    // start ping receiver
    pingRcvr.start(0, 100, 10*1024);
    // start telemetry
    eventLogger.start(0,98,120*1024);
    chanTlm.start(0,97,40*1024);
    prmDb.start(0,96,10*1024);
    // start driverTest (uses's file system)
    driverTest.start(0, 94, 40 * 1024);
    // start ADC
    adc.start(0,93,40*1024);
    // start FSWInfo (uses's file system)
    fswInfo.start(0, 92, 40 * 1024);
    //Idle task gets the lowest of priorities, consumes all free cycles
    idleTask.start(0xCAFECAFE, 4);
    idleTask.preamble();

    // Initialize socket server if and only if there is a valid specification
    //if (hostname != NULL && port_number != 0) {
    //    socketIpDriver.startSocketTask(100, 10 * 1024, hostname, port_number);
    //}
    return false;
}

void exitTasks(void) {
    rateGroup1Comp.exit();
    rateGroup2Comp.exit();
    rateGroup3Comp.exit();
    rateGroup4Comp.exit();
    fatalHandler.exit();
    cmdDisp.exit();
    eventLogger.exit();
    chanTlm.exit();
    prmDb.exit();
    fileUplink.exit();
    fileDownlink.exit();
    cmdSeq.exit();
    pingRcvr.exit();
    adc.exit();
    fswInfo.exit();
    driverTest.exit();
}

#ifdef TGT_OS_TYPE_VXWORKS
extern "C" {
    void go(void);
}
//VxWorks uses this go function to start
void go(void) {
    fatalHandler.registerHook();
    constructApp(false);

    // Start FPGA Driver
    Os::InterruptLock ilocker;
    FW_ASSERT(Os::TaskLock::lock() == 0);
    ilocker.lock();
#ifdef TGT_OS_TYPE_VXWORKS
    fpgaDriver.sphinx_time_rti_handlr((I32)&fpgaDriver);
#else
    fpgaDriver.sphinx_time_rti_handlr((I64)&fpgaDriver);
#endif
    ilocker.unLock();
    FW_ASSERT(Os::TaskLock::unLock() == 0);

    Os::Task::delay(10000);
}
#endif

