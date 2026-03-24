cmake_minimum_required(VERSION 3.10)
project(sudoDEM C CXX)

# Set CMP0177 policy to NEW to normalize install() DESTINATION paths
if(POLICY CMP0177)
 cmake_policy(SET CMP0177 NEW)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cMake")
find_package(Python 3.13 REQUIRED COMPONENTS Interpreter Development.Module Development.Embed)

INCLUDE(FindOpenMP)
INCLUDE(GetVersion)
include(GNUInstallDirs)


#===========================================================
# HACK!!! If the version of gcc is 4.8 or greater, we add -ftrack-macro-expansion=0
# and -save-temps into compiler to reduce the memory consumption during compilation.
# See http://bugs.debian.org/726009 for more information
# Can be removed later, if gcc fixes its regression
# Taken from http://stackoverflow.com/questions/4058565/check-gcc-minor-in-cmake

# EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
# IF (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
#  MESSAGE(STATUS "GCC Version >= 4.8. Adding -ftrack-macro-expansion=0 and -save-temps")
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrack-macro-expansion=0 -save-temps")
# ENDIF()

#===========================================================

IF ("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-512")
ENDIF()

#===========================================================

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-deprecated-declarations -Wno-undefined-var-template")
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")#sway

#===========================================================

IF (CMAKE_CXX_FLAGS)
 #If flags are set, add only neccessary flags
 IF (DEBUG)
  SET(CMAKE_VERBOSE_MAKEFILE 1)
  SET(CMAKE_BUILD_TYPE Debug)
  ADD_DEFINITIONS("-DSUDODEM_DEBUG")
 ELSE (DEBUG)
  SET(CMAKE_BUILD_TYPE Release)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
 ENDIF (DEBUG)
ELSE (CMAKE_CXX_FLAGS)
 #If flags are not set, add all useful flags
 IF (DEBUG)
  SET(CMAKE_VERBOSE_MAKEFILE 1)
  SET(CMAKE_BUILD_TYPE Debug)
  ADD_DEFINITIONS("-DSUDODEM_DEBUG")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fPIC -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security")
 ELSE (DEBUG)
  SET(CMAKE_BUILD_TYPE Release)
  SET(CMAKE_CXX_FLAGS "-Wall -fPIC -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security -s")
 ENDIF (DEBUG)
ENDIF (CMAKE_CXX_FLAGS)

#======================= THIRD-PARTY LIBRARIES =======================# 
# Add Cereal for serialization (header-only)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cereal/include)
# Add BZip2 and ZLIB for compression
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
MESSAGE(STATUS "BZip2 include dir: " ${BZIP2_INCLUDE_DIRS})
MESSAGE(STATUS "ZLIB include dir: " ${ZLIB_INCLUDE_DIRS})

#======================= PYBIND11 =======================#
# Add pybind11 support
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/pybind11" "${CMAKE_CURRENT_BINARY_DIR}/pybind11")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/pybind11/include")
MESSAGE(STATUS "Using pybind11 for Python bindings")

#===========================================================
# Find NumPy using modern Python module
find_package(Python REQUIRED COMPONENTS NumPy)
INCLUDE_DIRECTORIES(${Python_NumPy_INCLUDE_DIRS})

FIND_PACKAGE(BZip2 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
#===========================================================

SET(DEFAULT ON CACHE INTERNAL "Default value for enabled by default options")
SET(LINKLIBS "")
SET(CONFIGURED_FEATS "")
SET(DISABLED_FEATS "")

OPTION(NOSUFFIX "NOSUFFIX" ON)
OPTION(ENABLE_OPENMP "Enable OpenMP" ${DEFAULT})
OPTION(ENABLE_GUI "Enable GUI" ${DEFAULT})

#======================= EIGEN =======================#
set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})
MESSAGE(STATUS "Found Eigen3, at: ${EIGEN3_INCLUDE_DIR}")

IF (NOT VECTORIZE)
 MESSAGE(STATUS "Disable vectorization")
 ADD_DEFINITIONS("-DEIGEN_DONT_VECTORIZE -DEIGEN_DONT_ALIGN -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")
ELSE (NOT VECTORIZE)
 MESSAGE(STATUS "Enable vectorization")
ENDIF (NOT VECTORIZE)

#===========================================================
INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
SET(LINKLIBS "${LINKLIBS};${BZIP2_LIBRARIES};${ZLIB_LIBRARIES};")

#===========================================================
SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} Odeint")
ADD_DEFINITIONS("-DSUDODEM_ODEINT")

# Add OpenMP hints for macOS BEFORE find_package
IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Try common Homebrew prefixes (Apple Silicon vs Intel Macs)
  IF(EXISTS "/opt/homebrew/opt/libomp")
    SET(OPENMP_ROOT "/opt/homebrew/opt/libomp")
  ELSEIF(EXISTS "/usr/local/opt/libomp")
    SET(OPENMP_ROOT "/usr/local/opt/libomp")
  ENDIF()
  IF(OPENMP_ROOT)
    SET(OPENMP_INCLUDE_DIR "${OPENMP_ROOT}/include")
    SET(OPENMP_LIB_DIR "${OPENMP_ROOT}/lib")
    MESSAGE(STATUS "OpenMP root: ${OPENMP_ROOT}")
    # Set explicit hints for FindOpenMP (AppleClang uses -Xpreprocessor -fopenmp)
    SET(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_INCLUDE_DIR}")
    SET(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_INCLUDE_DIR}")
    SET(OpenMP_omp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
    SET(OpenMP_libomp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
    SET(OpenMP_C_LIB_NAMES "omp")
    SET(OpenMP_CXX_LIB_NAMES "omp")
  ENDIF()
ENDIF()

FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSUDODEM_OPENMP ${OpenMP_CXX_FLAGS}")
  MESSAGE(STATUS "Found OpenMP")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} OpenMP")
ELSE(OPENMP_FOUND)
  MESSAGE(STATUS "OpenMP NOT found")
  SET(ENABLE_OPENMP OFF)
  SET(DISABLED_FEATS "${DISABLED_FEATS} OPENMP")
ENDIF(OPENMP_FOUND)

#===========================================================

#======================= QT6/QT5 ====================================
MESSAGE(STATUS "Enable GUI: " ${ENABLE_GUI})
IF(ENABLE_GUI)
 # Try Qt6 first, fall back to Qt5
 FIND_PACKAGE(Qt6 CONFIG QUIET COMPONENTS Widgets OpenGL Xml OpenGLWidgets)
 IF(Qt6_FOUND)
  MESSAGE(STATUS "Found Qt6")
  SET(QT_VERSION_MAJOR 6)
  ADD_DEFINITIONS("-DQT_VERSION_MAJOR=6")
  SET(QT_FOUND TRUE)
  # Add QtOpenGLWidgets include path
  INCLUDE_DIRECTORIES(${Qt6OpenGLWidgets_INCLUDE_DIRS})
 ELSE()
  FIND_PACKAGE(Qt5 CONFIG REQUIRED Widgets Xml OpenGL)
  MESSAGE(STATUS "Found Qt5")
  SET(QT_VERSION_MAJOR 5)
  ADD_DEFINITIONS("-DQT_VERSION_MAJOR=5")
  SET(QT_FOUND TRUE)
 ENDIF()
 SET(QGLVIEWER_FOUND TRUE)
 FIND_PACKAGE(OpenGL)
 FIND_PACKAGE(GLUT)
 # Find glib2 using PkgConfig
 find_package(PkgConfig REQUIRED)
 pkg_check_modules(GLIB2 REQUIRED glib-2.0)


 # QGLViewer is now built as embedded static library in gui/qglviewer
 SET(QGLVIEWER_FOUND TRUE)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQGLVIEWER_FOUND")

 IF(QT_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND QGLVIEWER_FOUND)
  # Qt6 provides all OpenGL functionality, only link OpenGL library
  IF(QT_VERSION_MAJOR EQUAL 6)
   SET(GUI_LIBS ${OPENGL_LIBRARIES})
  ELSE()
   SET(GUI_LIBS ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
  ENDIF()
  SET(GUI_SRC_LIB "lib/opengl/GLUtils.cpp")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSUDODEM_OPENGL")
  #INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS})
  #INCLUDE_DIRECTORIES(${QT_INCLUDES})

  MESSAGE(STATUS "Found GUI-LIBS")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GUI")
 ELSE(QT_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND QGLVIEWER_FOUND)
  MESSAGE(STATUS "GUI-LIBS NOT found")
  MESSAGE(STATUS "qt ${QT_FOUND} opengl ${OPENGL_FOUND} glut ${GLUT_FOUND} qglviewer ${QGLVIEWER_FOUND}")
  SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
  SET(ENABLE_GUI OFF)
 ENDIF(QT_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND QGLVIEWER_FOUND)
ELSE(ENABLE_GUI)
 SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
ENDIF(ENABLE_GUI)
#===============================================

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#===========================================================

#===========================================================
SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../sudodeminstall/SudoDEM2D")
MESSAGE(STATUS "SudoDEM will be installed to ${CMAKE_INSTALL_PREFIX}")

#===========================================================
SET(SUDODEM_LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib/sudodem)
SET(SUDODEM_EXEC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
SET(SUDODEM_PY_PATH ${SUDODEM_LIB_PATH}/py)
SET(SUDODEM_DOC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc/sudodem${SUFFIX})
SET(SUDODEM_MAN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR})


SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


#===========================================================
IF(ENABLE_GUI)
  ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/gui")
ENDIF(ENABLE_GUI)
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/py")
#===========================================================

# this is only a temporary hack, headers should be installed in the build-dir directly
# perhaps there is a proper way, have to ask
IF(NOT EXISTS "${CMAKE_BINARY_DIR}/sudodem")
 EXECUTE_PROCESS(COMMAND ln -s ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/sudodem)
ENDIF ()
#===========================================================


FILE(GLOB QHULL_SRC_LIB "lib/qhull/*.c")
ADD_LIBRARY(qhull SHARED ${QHULL_SRC_LIB})
SET_TARGET_PROPERTIES(qhull PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(qhull)
INSTALL(TARGETS qhull DESTINATION ${SUDODEM_LIB_PATH})
#===========================================================
FILE(GLOB VORO_SRC_LIB "lib/voro++/voro++.cc")
ADD_LIBRARY(voro++ SHARED ${VORO_SRC_LIB})
SET_TARGET_PROPERTIES(voro++ PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(voro++)
INSTALL(TARGETS voro++ DESTINATION ${SUDODEM_LIB_PATH})
#===========================================================


FILE(GLOB SRC_CORE "core/*.cpp")
FILE(GLOB_RECURSE SRC_PKG "pkg/*.cpp")
FILE(GLOB SRC_LIB "lib/*.cpp")

SET(SRC_LIB "${SRC_LIB};lib/base/Math.cpp;lib/factory/ClassRegistryInstantiation.cpp")
SET(SRC_LIB "${SRC_LIB};lib/serialization/Serializable.cpp;lib/pyutil/gil.cpp;core/main/pyboot.cpp;${GUI_SRC_LIB};${CGAL_SRC_LIB}")

#===========================================================

IF (CHUNKSIZE)
 INCLUDE(CombineSources)
 COMBINE_SOURCES(${CMAKE_BINARY_DIR}/core "${SRC_CORE}" ${CHUNKSIZE})
 FILE(GLOB SRC_CORE_COMBINED "${CMAKE_BINARY_DIR}/core.*.cpp")
 COMBINE_SOURCES(${CMAKE_BINARY_DIR}/pkg "${SRC_PKG}" ${CHUNKSIZE})
 FILE(GLOB SRC_PKG_COMBINED "${CMAKE_BINARY_DIR}/pkg.*.cpp")
 COMBINE_SOURCES(${CMAKE_BINARY_DIR}/lib "${SRC_LIB}" ${CHUNKSIZE})
 FILE(GLOB SRC_LIB_COMBINED "${CMAKE_BINARY_DIR}/lib.*.cpp")
 ADD_LIBRARY(sudodem SHARED ${SRC_LIB_COMBINED} ${SRC_CORE_COMBINED} ${SRC_PKG_COMBINED})
ELSE (CHUNKSIZE)
 ADD_LIBRARY(sudodem SHARED ${SRC_CORE} ${SRC_PKG} ${SRC_LIB})
ENDIF (CHUNKSIZE)

#===========================================================
# Check for Python modules using execute_process
EXECUTE_PROCESS(
 COMMAND "${Python_EXECUTABLE}" "-c" "import minieigen; print(minieigen.__file__)"
 RESULT_VARIABLE _minieigen_status
 OUTPUT_VARIABLE _minieigen_location
 ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)
IF(_minieigen_status MATCHES 0)
 MESSAGE(STATUS "Use system minieigen version")
 SET(PY_minieigen TRUE)
ELSE()
 MESSAGE(STATUS "Use embedded version of minieigen. Please, consider installing the corresponding package")
 SET(PY_minieigen FALSE)
ENDIF()

EXECUTE_PROCESS(
 COMMAND "${Python_EXECUTABLE}" "-c" "import tkinter; print(tkinter.__file__)"
 RESULT_VARIABLE _tkinter_status
 ERROR_QUIET
)
IF(_tkinter_status MATCHES 0)
 SET(PY_tkinter TRUE)
ELSE()
 SET(PY_tkinter FALSE)
ENDIF()


#===========================================================


ADD_EXECUTABLE(sudodem2d ${CMAKE_CURRENT_SOURCE_DIR}/core/main/sudodem.cpp)
if(QT_VERSION_MAJOR EQUAL 6)
 set_target_properties(sudodem2d PROPERTIES AUTOMOC ON)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# Add libomp rpath when OpenMP is enabled
	if(OPENMP_FOUND AND OPENMP_LIB_DIR)
		SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@executable_path;@executable_path/../lib/sudodem;@executable_path/../lib/sudodem/py/sudodem;${OPENMP_LIB_DIR}")
	else()
		SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@executable_path;@executable_path/../lib/sudodem;@executable_path/../lib/sudodem/py/sudodem;")
	endif()
else()
	SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/sudodem/py/sudodem;$ORIGIN/../lib/3rdlibs;")
endif()

ADD_LIBRARY(boot SHARED ${CMAKE_CURRENT_SOURCE_DIR}/core/main/pyboot.cpp)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "" LINK_FLAGS "-Wl,-z,origin,--as-needed" INSTALL_RPATH "$ORIGIN;$ORIGIN/../../;$ORIGIN/../../../3rdlibs;") #add -z,origin
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "" INSTALL_RPATH "@loader_path;@loader_path/../../;@loader_path/../../../3rdlibs;")
else()
	SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "" INSTALL_RPATH "$ORIGIN;$ORIGIN/../../;$ORIGIN/../../../3rdlibs;")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	TARGET_LINK_LIBRARIES(sudodem qhull ${PYTHON_LIBRARIES} ${LINKLIBS} -lrt pybind11::module ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
else()
	# macOS - add libomp if OpenMP is enabled
	if(OPENMP_FOUND AND OPENMP_LIB_DIR)
		TARGET_LINK_LIBRARIES(sudodem qhull ${PYTHON_LIBRARIES} ${LINKLIBS} pybind11::module ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} "${OPENMP_LIB_DIR}/libomp.dylib")
	else()
		TARGET_LINK_LIBRARIES(sudodem qhull ${PYTHON_LIBRARIES} ${LINKLIBS} pybind11::module ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(sudodem PROPERTIES LINK_FLAGS "-Wl,--as-needed" INSTALL_RPATH "$ORIGIN;$ORIGIN/../3rdlibs;$ORIGIN/../3rdlibs/py;$ORIGIN/py/sudodem/qt" )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# Add libomp rpath when OpenMP is enabled
	if(OPENMP_FOUND AND OPENMP_LIB_DIR)
		SET_TARGET_PROPERTIES(sudodem PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../3rdlibs;@loader_path/../3rdlibs/py;@loader_path/py/sudodem/qt;@loader_path/../../../3rdlibs;${OPENMP_LIB_DIR}")
	else()
		SET_TARGET_PROPERTIES(sudodem PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../3rdlibs;@loader_path/../3rdlibs/py;@loader_path/py/sudodem/qt;@loader_path/../../../3rdlibs")
	endif()
else()
	SET_TARGET_PROPERTIES(sudodem PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../3rdlibs;$ORIGIN/../3rdlibs/py;$ORIGIN/py/sudodem/qt" )
endif()
TARGET_LINK_LIBRARIES(boot sudodem)
if(ENABLE_GUI)
 # Link boot against GLUT to resolve GLUT symbols needed by the Python package
 TARGET_LINK_LIBRARIES(boot ${GLUT_LIBRARIES})
endif()

TARGET_LINK_LIBRARIES(sudodem2d sudodem ${PYTHON_LIBRARIES} pybind11::embed pthread)

IF(ENABLE_GUI)
 TARGET_INCLUDE_DIRECTORIES(sudodem2d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gui)
 TARGET_LINK_LIBRARIES(sudodem2d _GLViewer ${GUI_LIBS})
 if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/sudodem;$ORIGIN/../lib/sudodem/py/sudodem;$ORIGIN/../lib/sudodem/py/sudodem/qt;$ORIGIN/../lib/3rdlibs" )
 elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Add libomp rpath when OpenMP is enabled
  if(OPENMP_FOUND AND OPENMP_LIB_DIR)
    SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@executable_path;@executable_path/../lib/sudodem;@executable_path/../lib/sudodem/py/sudodem;@executable_path/../lib/sudodem/py/sudodem/qt;@executable_path/../lib/3rdlibs;${OPENMP_LIB_DIR}")
  else()
    SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@executable_path;@executable_path/../lib/sudodem;@executable_path/../lib/sudodem/py/sudodem;@executable_path/../lib/sudodem/py/sudodem/qt;@executable_path/../lib/3rdlibs")
  endif()
 else()
  SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/sudodem;$ORIGIN/../lib/sudodem/py/sudodem;$ORIGIN/../lib/sudodem/py/sudodem/qt;$ORIGIN/../lib/3rdlibs" )
 endif()
ENDIF(ENABLE_GUI)


#====================================
#Back compatibility with scons
SET (realVersion ${SUDODEM_VERSION})
SET (version ${SUDODEM_VERSION})
SET (pyExecutable ${PYTHON_EXECUTABLE})
SET (profile "default")
SET (sourceRoot "${CMAKE_CURRENT_SOURCE_DIR}")
#====================================
CONFIGURE_FILE(core/main/sudodemcfg.h.in "${CMAKE_BINARY_DIR}/sudodemcfg.h")
CONFIGURE_FILE(py/__init__.py.in "${CMAKE_BINARY_DIR}/__init__.py")
#===========================================================

INSTALL(FILES "${CMAKE_BINARY_DIR}/__init__.py" DESTINATION ${SUDODEM_PY_PATH}/sudodem/)
INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/doc/sudodem-logo-note.png" DESTINATION "${SUDODEM_DOC_PATH}/img")

INSTALL(TARGETS sudodem DESTINATION ${SUDODEM_LIB_PATH})
INSTALL(TARGETS sudodem2d DESTINATION ${SUDODEM_EXEC_PATH}/)

#===========================================================
MESSAGE(STATUS "===========================================================")
MESSAGE(STATUS "SudoDEM configured with following features:${CONFIGURED_FEATS}")
MESSAGE(STATUS "Disabled features:${DISABLED_FEATS}")
IF (DEBUG)
 MESSAGE(STATUS "Debug build")
 SET (debugbuild " (debug build)")
ELSE (DEBUG)
 MESSAGE(STATUS "Optimized build")
ENDIF (DEBUG)
IF (CHUNKSIZE)
 MESSAGE(STATUS "CHUNKSIZE is set to " ${CHUNKSIZE})
ENDIF (CHUNKSIZE)
MESSAGE(STATUS "===========================================================")
#===========================================================
