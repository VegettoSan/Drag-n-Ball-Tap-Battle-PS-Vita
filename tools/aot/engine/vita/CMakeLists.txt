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

project(dbtb_original_engine LANGUAGES C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../..")
include("${REPO_ROOT}/cmake/LiveArea.cmake")
dbtb_prepare_livearea("${REPO_ROOT}" DBTB_LIVEAREA_DIR)
set(TEAVM_C_DIR "" CACHE PATH "Private TeaVM 0.12.3 generated C directory after patch_runtime.py")
if(NOT TEAVM_C_DIR OR NOT EXISTS "${TEAVM_C_DIR}/all.c")
  message(FATAL_ERROR "Pass -DTEAVM_C_DIR=/private/patched/c containing TeaVM all.c")
endif()
get_filename_component(TEAVM_C_DIR "${TEAVM_C_DIR}" ABSOLUTE)

set(VITA_APP_NAME "Dragon Ball Tap Battle Vita")
set(VITA_TITLEID "DBTB00001")
set(VITA_VERSION "00.26")
execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${REPO_ROOT}"
                OUTPUT_VARIABLE DBTB_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT DBTB_COMMIT)
  set(DBTB_COMMIT "unknown")
endif()

add_executable(${PROJECT_NAME}
  "${TEAVM_C_DIR}/all.c"
  "${REPO_ROOT}/tools/aot/engine/native/gles.cpp"
  "${REPO_ROOT}/tools/aot/engine/native/resources.cpp"
  "${REPO_ROOT}/tools/aot/engine/native/vita_platform.cpp"
  "${REPO_ROOT}/tools/aot/engine/native/vita_text.cpp"
  "${REPO_ROOT}/tools/aot/engine/native/vita_audio.cpp"
  "${REPO_ROOT}/tools/aot/engine/native/vita_compat.c"
  "${REPO_ROOT}/tools/aot/engine/vita/heap.c"
  "${REPO_ROOT}/tools/aot/vita/utf16.c"
  "${REPO_ROOT}/src/vfs.cpp"
  "${REPO_ROOT}/src/installed_data.cpp"
  "${REPO_ROOT}/src/pac.cpp"
  "${REPO_ROOT}/src/image.cpp"
  "${REPO_ROOT}/src/engine_resources.cpp"
  "${REPO_ROOT}/src/game_data.cpp"
  "${REPO_ROOT}/src/input.cpp"
  "${REPO_ROOT}/src/ui.cpp"
  "${REPO_ROOT}/src/log.cpp"
)

# Do not add TEAVM_C_DIR as a global -I path. TeaVM emits runtime headers named
# string.h/time.h; exposing that root to unrelated C++ sources shadows VitaSDK's
# libc headers. The generated all.c and its nested sources resolve their own
# quoted headers relative to the generated files.
target_include_directories(${PROJECT_NAME} PRIVATE
  "${REPO_ROOT}/tools/aot/engine/native"
  "${REPO_ROOT}/tools/aot/vita"
  "${REPO_ROOT}/src"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS YES
  CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS YES
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  _GNU_SOURCE=1
)
# Version/commit strings are only consumed by the Vita platform shim. Keeping
# them off the TeaVM amalgamation prevents a version bump from recompiling the
# complete ~24 MiB generated C translation unit.
set_source_files_properties(
  "${REPO_ROOT}/tools/aot/engine/native/vita_platform.cpp"
  PROPERTIES COMPILE_DEFINITIONS
    "DBTB_VERSION=\"${VITA_VERSION}\";DBTB_COMMIT=\"${DBTB_COMMIT}\""
)
target_compile_options(${PROJECT_NAME} PRIVATE
  -O2 -Wall -Wextra
  $<$<COMPILE_LANGUAGE:C>:-Wno-unused-function -Wno-unused-variable -Wno-unused-parameter>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>
)
# TeaVM 0.12.3 emits the complete reachable Java program as one ~24 MiB C
# amalgamation. -O2 exceeds the memory budget of modest builders; -O1 keeps
# useful optimization while native render/audio/platform code remains at -O2.
set_source_files_properties("${TEAVM_C_DIR}/all.c" PROPERTIES COMPILE_OPTIONS "-O1")
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-q,-z,max-page-size=0x10000")

# Dependencies are ordered from consumers to providers, matching vitaGL ports.
# pthread stays whole because libstdc++ runtime facilities use its weak wrappers;
# the native Vita audio worker itself deliberately avoids std::mutex/pthread locks.
target_link_libraries(${PROJECT_NAME}
  -Wl,--whole-archive pthread -Wl,--no-whole-archive
  stdc++
  vorbisfile vorbis ogg
  vitaGL png z
  vitashark SceShaccCgExt taihen_stub SceShaccCg_stub mathneon
  ScePvf_stub SceAudio_stub
  SceLibKernel_stub SceAppMgr_stub SceAppUtil_stub SceCommonDialog_stub
  SceGxm_stub SceDisplay_stub SceSysmodule_stub SceKernelDmacMgr_stub
  SceCtrl_stub SceTouch_stub SceIofilemgr_stub ScePower_stub m
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
vita_create_vpk(DBTapBattle-Vita-${VITA_VERSION}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION} NAME ${VITA_APP_NAME}
  FILE "${DBTB_LIVEAREA_DIR}/icon0.png" sce_sys/icon0.png
  FILE "${DBTB_LIVEAREA_DIR}/pic0.png" sce_sys/pic0.png
  FILE "${DBTB_LIVEAREA_DIR}/bg0.png" sce_sys/livearea/contents/bg0.png
  FILE "${DBTB_LIVEAREA_DIR}/startup.png" sce_sys/livearea/contents/startup.png
  FILE "${REPO_ROOT}/assets/livearea/template.xml" sce_sys/livearea/contents/template.xml
  FILE "${REPO_ROOT}/docs/THIRD_PARTY.md" notices/THIRD_PARTY.md
  FILE "${REPO_ROOT}/licenses/vitaGL-GPL-3.0.txt" notices/vitaGL-GPL-3.0.txt
  FILE "${REPO_ROOT}/licenses/vitaGL-LGPL-3.0.txt" notices/vitaGL-LGPL-3.0.txt
)
