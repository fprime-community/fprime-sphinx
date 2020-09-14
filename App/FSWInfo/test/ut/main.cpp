// ----------------------------------------------------------------------
// Main.cpp
// ----------------------------------------------------------------------

#include "Tester.hpp"
#include <fprime-sphinx-drivers/Util/SphinxDrvReg.hpp>

TEST(Test, Ping) {
    App::Tester tester;
    tester.testPing();
}

TEST(Test, PreambleEmitsVersionEVR) {
    App::Tester tester;
    tester.preambleEmitsVersionEVR();
}

TEST(Test, ReportFSWInfoPortEmitsEVR) {
    App::Tester tester;
    tester.reportFSWInfoPortEmitsEVR();
}

TEST(Test, ReportFSWInfoCommandEmitsEVR) {
    App::Tester tester;
    tester.reportFSWInfoCommandEmitsEVR();
}

TEST(Test, HandlesInitSRAMCommand) {
    App::Tester tester;
    tester.handlesInitSRAMCommand();
}

TEST(Test, HandlesResetFSWCommand) {
    App::Tester tester;
    tester.handlesResetFSWCommand();
}

TEST(Test, HandlesTimeCommands) {
    App::Tester tester;
    tester.handlesTimeCommands();
}

TEST(Test, HandlesFPGAWdogTimeLeftDmpCommand) {
    App::Tester tester;
    tester.handlesFPGAWdogTimeLeftDmpCommand();
}

TEST(Test, HandlesReadRegCommand) {
    App::Tester tester;
    tester.handlesReadRegCommand();
}

TEST(Test, HandlesWriteRegCommand) {
    App::Tester tester;
    tester.handlesWriteRegCommand();
}

TEST(Teardown, OK) {
    Drv::SphinxDrvReg::clearFiles();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
