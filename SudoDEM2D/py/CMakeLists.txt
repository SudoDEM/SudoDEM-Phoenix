#==================miniEigen=====================================
#find_python_module(minieigen)
#IF (PY_minieigen)
#  MESSAGE(STATUS "Use system minieigen version")
#ELSE (PY_minieigen)
#  MESSAGE(STATUS "Use embedded version of minieigen. Please, consider installing the corresponding package")
#  FILE(GLOB_RECURSE SRC_minieigen "${CMAKE_CURRENT_SOURCE_DIR}/mathWrap/*.cc")
#  ADD_LIBRARY(minieigen SHARED ${SRC_minieigen})
#  SET_TARGET_PROPERTIES(minieigen PROPERTIES PREFIX "")
#  TARGET_LINK_LIBRARIES(minieigen ${PYTHON_LIBRARIES})
#  INSTALL(TARGETS minieigen DESTINATION ${SUDODEM_PY_PATH})
#ENDIF (PY_minieigen)

#==================miniEigen=====================================

#==================pygts=========================================
# IF(ENABLE_GTS)
#   EXECUTE_PROCESS(
#     COMMAND "${Python_EXECUTABLE}" "-c" "import gts; print(gts.__file__)"
#     RESULT_VARIABLE _gts_status
#     ERROR_QUIET
#   )
#   IF(_gts_status MATCHES 0)
#     MESSAGE(STATUS "Use system gts version")
#     SET(PY_gts TRUE)
#   ELSE()
#     MESSAGE(STATUS "Use embedded version of gts. Please, consider installing the corresponding package")
#     SET(PY_gts FALSE)
#     FILE(GLOB SRC_PYGTS "${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/pygts-0.3.1/*.c")
#     ADD_LIBRARY(_gts SHARED ${SRC_PYGTS})
#     SET_TARGET_PROPERTIES(_gts PROPERTIES PREFIX "")
#     TARGET_LINK_LIBRARIES(_gts gts)
#     INSTALL(FILES 3rd-party/pygts-0.3.1/__init__.py DESTINATION ${SUDODEM_LIB_PATH}/py/gts)
#     INSTALL(FILES 3rd-party/pygts-0.3.1/pygts.py DESTINATION ${SUDODEM_LIB_PATH}/py/gts)
#     INSTALL(TARGETS _gts DESTINATION ${SUDODEM_LIB_PATH}/py/gts)
#   ENDIF()
# ENDIF(ENABLE_GTS)
#==================pygts=========================================

#==================pyModules=========================================

FILE(GLOB filesPY "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
INSTALL(FILES ${filesPY} DESTINATION ${SUDODEM_PY_PATH}/sudodem)

# Simplified library structure: merge pybind11 modules into wrapper
# Note: _polyhedra_utils is a pure Python module (no .cpp file)
ADD_LIBRARY(wrapper MODULE
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/sudodemWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/ClassRegistryAll.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WeightedAverage2d.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/customConverters.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_superellipse_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/svgobjects.cpp"
)

if (WIN32)
  TARGET_COMPILE_DEFINITIONS(wrapper PRIVATE NOMINMAX)
ENDIF()


# Set rpath based on platform
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	if(OPENMP_FOUND)
		SET_TARGET_PROPERTIES(wrapper PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}" INSTALL_RPATH "@loader_path;@loader_path/../../;@loader_path/../../../;@loader_path/../../../3rdlibs;@loader_path;${OpenMP_ROOT}/lib;${Python_RUNTIME_LIBRARY_DIRS};")
	else()
		SET_TARGET_PROPERTIES(wrapper PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}" INSTALL_RPATH "@loader_path;@loader_path/../../;@loader_path/../../../;@loader_path/../../../3rdlibs;@loader_path;${Python_RUNTIME_LIBRARY_DIRS};")
	endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(wrapper PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}" INSTALL_RPATH "$ORIGIN;$ORIGIN/../../;$ORIGIN/../../../;$ORIGIN/../../../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
elseif(WIN32)
  set_target_properties(wrapper PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}")
endif()

TARGET_LINK_LIBRARIES(wrapper sudodem voro++ pybind11::module)
INSTALL(TARGETS wrapper DESTINATION "${SUDODEM_PY_PATH}/sudodem/")
