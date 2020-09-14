# Add component subdirectories
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/PingReceiver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FSWInfo/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/IdleTask/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FatalHandler/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/DriverTest/")

# Add Ports
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports/FSWInfo_Generate/")

