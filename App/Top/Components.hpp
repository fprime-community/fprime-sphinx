#ifndef __LITS_COMPONENTS_HEADER__
#define __LITS_COMPONENTS_HEADER__
#include <Svc/ActiveRateGroup/ActiveRateGroupImpl.hpp>
#include <Svc/RateGroupDriver/RateGroupDriverImpl.hpp>

#include <Svc/CmdDispatcher/CommandDispatcherImpl.hpp>
#include <Svc/CmdSequencer/CmdSequencerImpl.hpp>
#include <Svc/PassiveConsoleTextLogger/ConsoleTextLoggerImpl.hpp>
#include <Svc/ActiveLogger/ActiveLoggerImpl.hpp>
#include <Svc/TlmChan/TlmChanImpl.hpp>
#include <Svc/PrmDb/PrmDbImpl.hpp>
#include <Fw/Obj/SimpleObjRegistry.hpp>
#include <Svc/FileUplink/FileUplink.hpp>
#include <Svc/FileDownlink/FileDownlink.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <Svc/Health/HealthComponentImpl.hpp>

#include <App/PingReceiver/PingReceiverComponentImpl.hpp>

#include <fprime-sphinx-drivers/UartDriver/UartDriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/FPGADriver/FPGADriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/SphinxTime/SphinxTimeComponentImpl.hpp>
#include <fprime-sphinx-drivers/ADC/ADCComponentImpl.hpp>
#include <fprime-sphinx-drivers/TrapRegistry/TrapRegistry.hpp>
#include <fprime-sphinx-drivers/SPIDriverGeneric/SPIDriverGenericComponentImpl.hpp>
#include <fprime-sphinx-drivers/FpgaGpioDriver/FpgaGpioDriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/GPIODriver/GPIODriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/GPIOInterruptRouter/GPIOInterruptRouter.hpp>
#include <fprime-sphinx-drivers/SphinxUartOnboardDriver/SphinxUartOnboardDriverComponentImpl.hpp>
#include <fprime-sphinx-drivers/FPGASPIDriver/FPGASPIDriverComponentImpl.hpp>

#include <App/IdleTask/IdleTaskComponentImpl.hpp>
#include <App/FSWInfo/FSWInfoComponentImpl.hpp>
#include <App/FatalHandler/FatalHandlerComponentImpl.hpp>
#include <App/DriverTest/DriverTestComponentImpl.hpp>

#include <Svc/GroundInterface/GroundInterface.hpp>

void constructAppArchitecture(void);
bool constructApp(bool dump);
void exitTasks(void);


extern Svc::RateGroupDriverImpl rateGroupDriverComp;
extern Svc::ActiveRateGroupImpl rateGroup1Comp, rateGroup2Comp, rateGroup3Comp, rateGroup4Comp;
extern Svc::CmdSequencerComponentImpl cmdSeq;
extern Svc::GroundInterfaceComponentImpl groundIf;
extern Svc::ConsoleTextLoggerImpl textLogger;
extern Svc::ActiveLoggerImpl eventLogger;
extern Svc::TlmChanImpl chanTlm;
extern Svc::CommandDispatcherImpl cmdDisp;
extern Svc::PrmDbImpl prmDb;
extern Svc::FileUplink fileUplink;
extern Svc::FileDownlink fileDownlink;
extern Svc::BufferManager fileDownlinkBufferManager;
extern Svc::BufferManager fileUplinkBufferManager;
extern Svc::BufferManager bufferManagerFSWImageManager;
extern Svc::HealthImpl health;

extern App::PingReceiverComponentImpl pingRcvr;
extern Drv::UartDriverComponentImpl uartDriver;
extern Drv::FPGADriverComponentImpl fpgaDriver;
extern Drv::SphinxTimeComponentImpl sphinxTime;
extern Drv::ADCComponentImpl adc;
extern Drv::TrapRegistry trap_registrar;
extern Drv::SPIDriverGenericComponentImpl spiDriverGeneric;
extern Drv::FPGASPIDriverComponentImpl fpgaSpiDriver;
extern Drv::FpgaGpioDriverComponentImpl fpgaGpioDriver;
extern Drv::GPIODriverComponentImpl gpioDriver;
extern Drv::GPIOInterruptRouter gpioIntRouter;
extern Drv::SphinxUartOnboardDriverComponentImpl hw_uart;
extern Drv::SphinxUartOnboardDriverComponentImpl fw_uart;

extern App::IdleTaskComponentImpl idleTask;
extern App::FSWInfoComponentImpl fswInfo;
extern App::FatalHandlerComponentImpl fatalHandler;
extern App::DriverTestComponentImpl driverTest;

#endif
