# LinuxDeploy.cmake - Deploy non-Qt runtime dependencies for Linux standalone package
#
# This script copies non-Qt runtime libraries (zlib, bz2, OpenMP) to the
# install directory. Qt deployment is handled separately by qt_generate_deploy_app_script.
#
# Usage:
#   cmake -DSUDODEM_LIB_PATH=... -DCONDA_PREFIX_CMAKE=... -P LinuxDeploy.cmake
#
# Required variables:
#   SUDODEM_LIB_PATH  - Path to library directory
#   CONDA_PREFIX_CMAKE - Conda prefix path
#   OPENMP_FOUND      - Whether OpenMP was found (TRUE/FALSE)
#   CMAKE_INSTALL_PREFIX - Install prefix path

cmake_minimum_required(VERSION 3.16)

message(STATUS "===========================================================")
message(STATUS "Linux Package Deployment (Non-Qt Libraries)")
message(STATUS "===========================================================")
message(STATUS "Library path: ${SUDODEM_LIB_PATH}")
message(STATUS "Conda prefix: ${CONDA_PREFIX_CMAKE}")
message(STATUS "OpenMP found: ${OPENMP_FOUND}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

#======================= RUNTIME LIBRARIES =======================#
message(STATUS "Copying runtime libraries from conda...")

# Copy common runtime libraries from conda if they exist
foreach(_lib zlib libbz2)
  # Copy unversioned .so
  if(EXISTS "${CONDA_PREFIX_CMAKE}/lib/${_lib}.so")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CONDA_PREFIX_CMAKE}/lib/${_lib}.so"
        "${SUDODEM_LIB_PATH}/"
      RESULT_VARIABLE _copy_result
    )
    if(_copy_result EQUAL 0)
      message(STATUS "  Copied: ${_lib}.so")
    else()
      message(WARNING "  Failed to copy: ${_lib}.so")
    endif()
  endif()

  # Copy versioned .so.1
  if(EXISTS "${CONDA_PREFIX_CMAKE}/lib/${_lib}.so.1")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CONDA_PREFIX_CMAKE}/lib/${_lib}.so.1"
        "${SUDODEM_LIB_PATH}/"
      RESULT_VARIABLE _copy_result
    )
    if(_copy_result EQUAL 0)
      message(STATUS "  Copied: ${_lib}.so.1")
    else()
      message(WARNING "  Failed to copy: ${_lib}.so.1")
    endif()
  endif()
endforeach()

#======================= OPENMP =======================#
if(OPENMP_FOUND)
  message(STATUS "Copying OpenMP library...")

  # Common locations for libgomp on Linux
  set(OPENMP_LIB_PATHS
    "/usr/lib/x86_64-linux-gnu/libgomp.so.1"
    "/usr/lib/libgomp.so.1"
    "/usr/lib64/libgomp.so.1"
    "/lib/x86_64-linux-gnu/libgomp.so.1"
  )

  foreach(_omp_path IN LISTS OPENMP_LIB_PATHS)
    if(EXISTS "${_omp_path}")
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_omp_path}"
          "${SUDODEM_LIB_PATH}/"
        RESULT_VARIABLE _copy_result
      )
      if(_copy_result EQUAL 0)
        message(STATUS "  Copied: libgomp.so.1 from ${_omp_path}")
        break()
      endif()
    endif()
  endforeach()
else()
  message(STATUS "OpenMP not found, skipping")
endif()

#======================= SUMMARY =======================#
message(STATUS "===========================================================")
message(STATUS "Linux package deployment complete!")
message(STATUS "Note: Qt dependencies deployed via qt_generate_deploy_app_script")
message(STATUS "===========================================================")
