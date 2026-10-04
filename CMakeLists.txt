cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED VITASDK)
    set(CMAKE_TOOLCHAIN_FILE "${VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  elseif(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Set VITASDK or CMAKE_TOOLCHAIN_FILE to a real VitaSDK installation")
  endif()
endif()
project(dbtb_vita LANGUAGES C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Dragon Ball Tap Battle Vita")
set(VITA_TITLEID "DBTB00001")
set(VITA_VERSION "00.02")
execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE DBTB_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT DBTB_COMMIT)
  set(DBTB_COMMIT "unknown")
endif()

add_executable(${PROJECT_NAME} src/main.cpp src/vfs.cpp src/pac.cpp src/ui.cpp src/image.cpp src/log.cpp src/input.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE src)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS YES)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2 $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions> $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-q")
target_compile_definitions(${PROJECT_NAME} PRIVATE DBTB_VERSION="${VITA_VERSION}" DBTB_COMMIT="${DBTB_COMMIT}")

# Match vitaGL sample static dependency order: dependants precede providers.
# PNG decoding needs png/z; the bootstrap does not use libzip.
target_link_libraries(${PROJECT_NAME}
  vitaGL png z
  vitashark SceShaccCgExt taihen_stub SceShaccCg_stub mathneon
  SceLibKernel_stub SceAppMgr_stub SceAppUtil_stub SceCommonDialog_stub
  SceGxm_stub SceDisplay_stub SceSysmodule_stub SceKernelDmacMgr_stub
  SceCtrl_stub SceTouch_stub SceIofilemgr_stub m
)
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION} NAME ${VITA_APP_NAME}
  FILE docs/THIRD_PARTY.md notices/THIRD_PARTY.md
  FILE licenses/vitaGL-GPL-3.0.txt notices/vitaGL-GPL-3.0.txt
  FILE licenses/vitaGL-LGPL-3.0.txt notices/vitaGL-LGPL-3.0.txt)
