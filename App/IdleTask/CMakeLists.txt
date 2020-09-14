####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
set(MOD_DEPS
    fprime-sphinx-drivers/Util
    Os
    Utils
    )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Sphinx-VxWorks6")
    list(APPEND MOD_DEPS
        "fprime-vxworks/VxWorks"
    )
endif()

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/IdleTaskComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/IdleTaskPwrOnResetCntSerializableAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/IdleTaskComponentImpl.cpp"
)

register_fprime_module()

set(UT_MOD_DEPS
    fprime-sphinx-drivers/Util/LinuxDrvReg
    fprime-sphinx-drivers/Util
    Os
    Utils
    )

### UTs ###
set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/IdleTaskComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
)
register_fprime_ut()
