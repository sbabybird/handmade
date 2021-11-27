cmake_minimum_required(VERSION 3.0.0)
project(handmade VERSION 0.1.0)

# include(CTest)
# enable_testing()

# set(CMAKE_EXE_LINKER_FLAGS"${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup")

link_libraries(user32.lib gdi32.lib)

add_executable(handmade WIN32 win32_handmade.cpp trace.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
