// ----------------------------------------------------------------------
// Main.cpp
// ----------------------------------------------------------------------

#include "Tester.hpp"

  //UT: does a fatal call actually log a fatal event that can be downlinked?
  //UT: does it produce a Fatal when the fatal command is called
  //UT: do late events discard


TEST(Test, TestClearMemory) {
    App::Tester tester;
    tester.clear_space();
}

TEST(Test, TestFatalDownlink) {
    App::Tester tester;
    tester.fatal_log();
}

TEST(Test, TestFatalDropOnOverflow) {
    App::Tester tester;
    tester.fatal_overflow_drop();
}

TEST(Test, TestFullWarning) {
    App::Tester tester;
    tester.full_warning();
}

TEST(Test, TestPing) {
    App::Tester tester;
    tester.ping_test();
}

TEST(Test, TestSmallRegion) {
    App::Tester tester;
    tester.small_region_drop();
}

TEST(Test, TestMemNotInitialized) {
    App::Tester tester;
    tester.memory_not_initialized();
}

TEST(Test, TestMemCorrupt) {
    App::Tester tester;
    tester.memory_corrupt();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
