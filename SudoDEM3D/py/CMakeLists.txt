#==================miniEigen=====================================
#find_python_module(minieigen)
#IF (PY_minieigen)
#  MESSAGE(STATUS "Use system minieigen version")
#ELSE (PY_minieigen)
#  MESSAGE(STATUS "Use embedded version of minieigen. Please, consider installing the corresponding package")
#  FILE(GLOB_RECURSE SRC_minieigen "${CMAKE_CURRENT_SOURCE_DIR}/mathWrap/*.cc")
#  ADD_LIBRARY(minieigen SHARED ${SRC_minieigen})
#  SET_TARGET_PROPERTIES(minieigen PROPERTIES PREFIX "")
#  TARGET_LINK_LIBRARIES(minieigen ${Python_LIBRARIES})
#  INSTALL(TARGETS minieigen DESTINATION ${SUDODEM_PY_PATH})
#ENDIF (PY_minieigen)

#==================miniEigen=====================================

#==================pyModules=========================================

FILE(GLOB filesPY "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
INSTALL(FILES ${filesPY} DESTINATION ${SUDODEM_PY_PATH}/sudodem)

# Simplified library structure: merge pybind11 modules into wrapper
ADD_LIBRARY(wrapper MODULE
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/sudodemWrapper.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/ClassRegistryAll.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/wrapper/customConverters.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_superquadrics_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_fem_utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/_gjkparticle_utils.cpp"
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
