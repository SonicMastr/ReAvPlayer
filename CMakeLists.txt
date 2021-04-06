cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(reAvPlayer LANGUAGES C)

include(VitaDevelopmentSuite)
set(CMAKE_C_STANDARD 99)

add_compile_options(
  -Xdiag=0 -Xquit=2 -O3
)

add_link_options(
  -s
)

include_directories(
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(${PROJECT_NAME}.suprx MODULE
  main.c
)

target_link_libraries(${PROJECT_NAME}.suprx
  taihen_stub_weak
  SceVideodecUser_stub_weak
  SceSysmodule_stub_weak
  SceDbg_stub
)