cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your VitaSDK path")
  endif()
endif()

project(dbtb_vita LANGUAGES C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Dragon Ball Tap Battle Vita")
set(VITA_TITLEID "DBTB00001")
set(VITA_VERSION "00.01")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -Wl,-q")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -Wl,-q -std=gnu++14 -fno-exceptions -fno-rtti")

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/vfs.cpp
  src/pac.cpp
  src/ui.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
  vitaGL
  SceLibKernel_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceCommonDialog_stub
  SceGxm_stub
  SceDisplay_stub
  SceSysmodule_stub
  SceKernelDmacMgr_stub
  SceCtrl_stub
  SceIofilemgr_stub
  SceShaccCgExt
  SceShaccCg_stub
  taihen_stub
  vitashark
  mathneon
  zip
  z
  m
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
