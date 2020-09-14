####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/AppTopologyAppAi.xml"
  "${CMAKE_CURRENT_LIST_DIR}/Topology.cpp"  
  "${CMAKE_CURRENT_LIST_DIR}/GPIODefATB.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/FPGAGPIODefATB.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"  
)
# Note: supply non-explicit dependencies here
set(MOD_DEPS
  Fw/Logger
  fprime-sphinx-drivers/Util
  fprime-sphinx-drivers/TrapRegistry
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Sphinx-VxWorks6")
    list(APPEND MOD_DEPS
        "fprime-vxworks/VxWorks"
    )
endif()

register_fprime_executable()
