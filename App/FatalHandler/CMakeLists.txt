####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FatalHandlerComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/FatalHandlerComponentImpl.cpp"
)

set(MOD_DEPS
    fprime-sphinx-drivers/Ports/SphinxTimeVal
    fprime-sphinx-drivers/Util
    Utils
    Utils/Hash
    fprime-sphinx-drivers/FPGADriver
    App/IdleTask
    Svc/Fatal
   )

register_fprime_module()

set(UT_MOD_DEPS
        fprime-sphinx-drivers/Util/LinuxDrvReg
        fprime-sphinx-drivers/Random
        fprime-sphinx-drivers/Util
        )

set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FatalHandlerComponentAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tester.cpp"
  )
register_fprime_ut()
