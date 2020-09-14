// ----------------------------------------------------------------------
// Main.cpp
// ----------------------------------------------------------------------

#include "Tester.hpp"

TEST(Test, TestNoCorrupt) {
    App::Tester tester;
    tester.test_corruption();
}
TEST(Test, TestStartTelemeter) {
    App::Tester tester;
    tester.test_start_telem();
}
TEST(Test, TestChangeTelemeter) {
    App::Tester tester;
    tester.test_change_telem();
}
TEST(Test, TestChecksumInit) {
    App::Tester tester;
    tester.test_checksum_init();
}
TEST(Test, TestHardResetInit) {
    App::Tester tester;
    tester.test_reset_init();
}
TEST(Test, TestUninit) {
    App::Tester tester;
    tester.test_uninit();
}
TEST(Test, TestSignleBitErrors) {
    App::Tester tester;
    tester.test_detect_sbes_errors();
}
TEST(Test, TestDoubleBitErrors) {
    App::Tester tester;
    tester.test_detect_dbes_errors();
}
TEST(Test, TestPreambleLoadFail) {
    App::Tester tester;
    tester.test_preamble_load_fail();
}
TEST(Test, TestCommands) {
    App::Tester tester;
    tester.test_cmds();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
