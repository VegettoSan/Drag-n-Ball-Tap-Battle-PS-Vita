# Shared, fail-closed LiveArea preparation for every Vita package target.
# The approved PNGs are stored as base64 text to prevent accidental image
# re-encoding by repository tooling. They are reconstructed byte-for-byte and
# validated before vita_create_vpk sees them.

if(NOT DEFINED DBTB_LIVEAREA_HELPER_INCLUDED)
  set(DBTB_LIVEAREA_HELPER_INCLUDED TRUE)

  find_program(DBTB_PYTHON_EXECUTABLE NAMES python3 python)
  if(NOT DBTB_PYTHON_EXECUTABLE)
    message(FATAL_ERROR "Python 3 is required to validate/reconstruct LiveArea assets")
  endif()

  function(dbtb_prepare_livearea repo_root out_var)
    set(_source "${repo_root}/assets/livearea")
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/livearea-assets")

    execute_process(
      COMMAND "${DBTB_PYTHON_EXECUTABLE}"
              "${repo_root}/tools/decode_livearea_assets.py"
              --source "${_source}"
              --output "${_output}"
      RESULT_VARIABLE _rc
      OUTPUT_VARIABLE _stdout
      ERROR_VARIABLE _stderr
    )

    if(NOT _rc EQUAL 0)
      message(FATAL_ERROR
        "LiveArea asset validation/reconstruction failed (exit ${_rc}):\n"
        "${_stdout}\n${_stderr}")
    endif()

    string(STRIP "${_stdout}" _stdout)
    if(_stdout)
      message(STATUS "LiveArea assets:\n${_stdout}")
    endif()

    set(${out_var} "${_output}" PARENT_SCOPE)
  endfunction()
endif()
