####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####

set(MOD_DEPS
    fprime-sphinx-drivers/Util
    )

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Tools/gen_version.py ${CMAKE_CURRENT_LIST_DIR}/version.c
    )

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FSWInfoComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/FSWInfoComponentImpl.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/version.c"
)

register_fprime_module()

set(UT_MOD_DEPS
    fprime-sphinx-drivers/Util/LinuxDrvReg
    fprime-sphinx-drivers/Util
    )

set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FSWInfoComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/version_stub.c"
  )
register_fprime_ut()
