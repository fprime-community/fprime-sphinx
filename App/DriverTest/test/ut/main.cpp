// ----------------------------------------------------------------------
// TestMain.cpp
// ----------------------------------------------------------------------

#include "Tester.hpp"

TEST(Nominal, Ping) {
    App::Tester tester;
    tester.ping();
}

TEST(Nominal, SchedIn) {
    App::Tester tester;
    tester.schedIn();
}

TEST(Nominal, SpiReadWrite) {
    App::Tester tester;
    tester.spiReadWrite();
}

TEST(Nominal, FpgaSpiReadWrite) {
    App::Tester tester;
    tester.fpgaSpiReadWrite();
}

TEST(Nominal, GpioSet) {
    App::Tester tester;
    tester.gpioSet();
}

TEST(Nominal, FpgaGpioSet) {
    App::Tester tester;
    tester.fpgaGpioSet();
}

TEST(Nominal, HwUartRead) {
    App::Tester tester;
    tester.hwUartRead();
}

TEST(Nominal, HwUartWrite) {
    App::Tester tester;
    tester.hwUartWrite();
}

TEST(Nominal, FwUartRead) {
    App::Tester tester;
    tester.fwUartRead();
}

TEST(Nominal, FwUartWrite) {
    App::Tester tester;
    tester.fwUartWrite();
}

TEST(Fail, SpiReadWrite) {
    App::Tester tester;
    tester.spiReadWriteFail();
}

TEST(Fail, FpgaSpiReadWrite) {
    App::Tester tester;
    tester.fpgaSpiReadWriteFail();
}

TEST(Fail, GpioSet) {
    App::Tester tester;
    tester.gpioSetFail();
}

TEST(Fail, FpgaGpioSet) {
    App::Tester tester;
    tester.fpgaGpioSetFail();
}

TEST(Fail, HwUartRead) {
    App::Tester tester;
    tester.hwUartReadFail();
}

TEST(Fail, HwUartWrite) {
    App::Tester tester;
    tester.hwUartWriteFail();
}

TEST(Fail, FwUartRead) {
    App::Tester tester;
    tester.fwUartReadFail();
}

TEST(Fail, FwUartWrite) {
    App::Tester tester;
    tester.fwUartWriteFail();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
