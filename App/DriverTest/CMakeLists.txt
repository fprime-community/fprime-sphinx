####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/DriverTestComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/DriverTestComponentImpl.cpp"
)

set(MOD_DEPS
  fprime-sphinx-drivers/SphinxUartOnboardDriver
  fprime-sphinx-drivers/FPGASPIDriver
  fprime-sphinx-drivers/SPIDriverGeneric
  fprime-sphinx-drivers/GPIODriver
  fprime-sphinx-drivers/FpgaGpioDriver
  )

register_fprime_module()

set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/DriverTestComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
  )
register_fprime_ut()
