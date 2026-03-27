cmake_minimum_required(VERSION 3.16)
project(sudoDEM C CXX)

set(BUILD_STANDALONE_WIN ON CACHE BOOL "Standalone program WIN")
set(BUILD_STANDALONE_LINUX ON CACHE BOOL "Standalone program LINUX")
set(BUILD_STANDALONE_MACOS ON CACHE BOOL "Standalone program MACOS")

# Set CMP0177 policy to NEW to normalize install() DESTINATION paths
if(POLICY CMP0177)
 cmake_policy(SET CMP0177 NEW)
endif()

file(TO_CMAKE_PATH "$ENV{CONDA_PREFIX}" CONDA_PREFIX_CMAKE)
list(PREPEND CMAKE_PREFIX_PATH "${CONDA_PREFIX_CMAKE}")
MESSAGE(STATUS "Miniconda env: ${CONDA_PREFIX_CMAKE}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cMake")

if(WIN32)
  set(Python_EXECUTABLE "${CONDA_PREFIX_CMAKE}/python.exe" CACHE FILEPATH "Python executable" FORCE)
else()
  set(Python_EXECUTABLE "${CONDA_PREFIX_CMAKE}/bin/python" CACHE FILEPATH "Python executable" FORCE)
endif()

find_package(Python 3.13 REQUIRED COMPONENTS Interpreter Development.Module Development.Embed)

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
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-512 -Wno-undefined-var-template -Wno-inconsistent-missing-override")
ENDIF()

#===========================================================

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/Zc:preprocessor)
  add_compile_options(/bigobj)
  add_compile_options(/utf-8)
else(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()


IF (CMAKE_CXX_FLAGS)
  IF (DEBUG)
    SET(CMAKE_VERBOSE_MAKEFILE 1)
    SET(CMAKE_BUILD_TYPE Debug)
    ADD_DEFINITIONS("-DSUDODEM_DEBUG")
  ELSE (DEBUG)
    SET(CMAKE_BUILD_TYPE Release)
  ENDIF (DEBUG)
ELSE (CMAKE_CXX_FLAGS)
  IF (DEBUG)
    SET(CMAKE_VERBOSE_MAKEFILE 1)
    SET(CMAKE_BUILD_TYPE Debug)
    ADD_DEFINITIONS("-DSUDODEM_DEBUG")
    IF (MSVC)
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    ELSE()
      SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fPIC -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security")
    ENDIF()
  ELSE (DEBUG)
    SET(CMAKE_BUILD_TYPE Release)
    IF (MSVC)
      SET(CMAKE_CXX_FLAGS "/O2")
    ELSE()
      SET(CMAKE_CXX_FLAGS "-Wall -fPIC -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security -s")
    ENDIF()
  ENDIF (DEBUG)
ENDIF (CMAKE_CXX_FLAGS)

#======================= THIRD-PARTY LIBRARIES =======================# 
# Add Cereal for serialization (header-only)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cereal/include)
# Add BZip2 and ZLIB for compression



if(WIN32)
  set(BZIP2_INCLUDE_DIR "${CONDA_PREFIX_CMAKE}/Library/include")
  set(BZIP2_LIBRARY_RELEASE "${CONDA_PREFIX_CMAKE}/Library/lib/bzip2.lib")
  set(ZLIB_ROOT "${CONDA_PREFIX_CMAKE}/Library")
else(WIN32)
  set(BZIP2_INCLUDE_DIR "${CONDA_PREFIX_CMAKE}/include")
  set(ZLIB_ROOT "${CONDA_PREFIX_CMAKE}")
endif(WIN32)


find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
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
INCLUDE_DIRECTORIES(${Python_INCLUDE_DIRS})

#===========================================================

SET(DEFAULT ON CACHE INTERNAL "Default value for enabled by default options")
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
INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})

#===========================================================
SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} Odeint")
ADD_DEFINITIONS("-DSUDODEM_ODEINT")


# # Add OpenMP hints for macOS BEFORE find_package
# IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#   # Try common Homebrew prefixes (Apple Silicon vs Intel Macs)
#   IF(EXISTS "/opt/homebrew/opt/libomp")
#     SET(OPENMP_ROOT "/opt/homebrew/opt/libomp")
#   ELSEIF(EXISTS "/usr/local/opt/libomp")
#     SET(OPENMP_ROOT "/usr/local/opt/libomp")
#   ENDIF()
#   IF(OPENMP_ROOT)
#     SET(OPENMP_INCLUDE_DIR "${OPENMP_ROOT}/include")
#     SET(OPENMP_LIB_DIR "${OPENMP_ROOT}/lib")
#     MESSAGE(STATUS "OpenMP root: ${OPENMP_ROOT}")
#     # Set explicit hints for FindOpenMP (AppleClang uses -Xpreprocessor -fopenmp)
#     SET(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_INCLUDE_DIR}")
#     SET(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_INCLUDE_DIR}")
#     SET(OpenMP_omp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
#     SET(OpenMP_libomp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
#     SET(OpenMP_C_LIB_NAMES "omp")
#     SET(OpenMP_CXX_LIB_NAMES "omp")
#   ENDIF()
# ENDIF()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(DEFINED ENV{OpenMP_ROOT} AND NOT "$ENV{OpenMP_ROOT}" STREQUAL "")
      message(STATUS "Env OpenMP_ROOT exists: $ENV{OpenMP_ROOT}")
  else()
      message(STATUS "Env OpenMP_ROOT not set, try /opt/homebrew/opt/libomp on macos!")
      set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
  endif()
endif()

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
  # Add Qt6 include paths
  INCLUDE_DIRECTORIES(${Qt6Widgets_INCLUDE_DIRS} ${Qt6OpenGL_INCLUDE_DIRS} ${Qt6OpenGLWidgets_INCLUDE_DIRS} ${Qt6Xml_INCLUDE_DIRS})
 ELSE()
  # Fall back to Qt5
  FIND_PACKAGE(Qt5 CONFIG REQUIRED Widgets Xml OpenGL)
  FIND_PACKAGE(OpenGL)
  FIND_PACKAGE(GLUT)
  # Find glib2 using PkgConfig (only needed for Qt5)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GLIB2 REQUIRED glib-2.0)
  IF(Qt5_FOUND AND OPENGL_FOUND AND GLUT_FOUND)
    SET(QT_VERSION_MAJOR 5)
    ADD_DEFINITIONS("-DQT_VERSION_MAJOR=5")
    SET(QT_FOUND TRUE)
  ELSE()
    SET(QT_FOUND FALSE)
  ENDIF()
 ENDIF()


 # QGLViewer is now built as embedded static library in gui/qglviewer
 SET(QGLVIEWER_FOUND TRUE)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQGLVIEWER_FOUND")

FIND_PACKAGE(OpenGL)

if(WIN32)
  SET(FreeGLUT_DIR "${CMAKE_SOURCE_DIR}/../thirdparty/freeglut_win64/lib/cmake/FreeGLUT")
  find_package(FreeGLUT CONFIG REQUIRED)
  set(SUDODEM_GLUT_TARGET FreeGLUT::freeglut)
else(WIN32)
  find_package(GLUT REQUIRED)
  set(SUDODEM_GLUT_TARGET GLUT::GLUT)
endif(WIN32)


 IF(QT_VERSION_MAJOR EQUAL 6)
   	IF(QT_FOUND AND OPENGL_FOUND AND QGLVIEWER_FOUND)
             SET(GUI_LIBS Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets ${OPENGL_LIBRARIES} ${SUDODEM_GLUT_TARGET})
             SET(GUI_SRC_LIB "lib/opengl/GLUtils.cpp")
             SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSUDODEM_OPENGL")
             MESSAGE(STATUS "Found GUI-LIBS (Qt6)")
             SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GUI")
   ELSE()
     MESSAGE(STATUS "GUI-LIBS NOT found")
     MESSAGE(STATUS "qt ${QT_FOUND} opengl ${OPENGL_FOUND} qglviewer ${QGLVIEWER_FOUND}")
     SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
     SET(ENABLE_GUI OFF)
   ENDIF()
 ELSE()
   IF(QT_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND QGLVIEWER_FOUND)
     SET(GUI_LIBS ${SUDODEM_GLUT_TARGET} ${OPENGL_LIBRARIES})
     SET(GUI_SRC_LIB "lib/opengl/GLUtils.cpp")
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSUDODEM_OPENGL")
     MESSAGE(STATUS "Found GUI-LIBS (Qt5)")
     SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GUI")
   ELSE()
     MESSAGE(STATUS "GUI-LIBS NOT found")
     MESSAGE(STATUS "qt ${QT_FOUND} opengl ${OPENGL_FOUND} glut ${GLUT_FOUND} qglviewer ${QGLVIEWER_FOUND}")
     SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
     SET(ENABLE_GUI OFF)
   ENDIF()
 ENDIF()
ELSE(ENABLE_GUI)
 SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
ENDIF(ENABLE_GUI)
#===============================================

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#===========================================================

#===========================================================
SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../SudoDEM2D_install")
MESSAGE(STATUS "SudoDEM will be installed to ${CMAKE_INSTALL_PREFIX}")

#===========================================================
SET(SUDODEM_LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib/sudodem)
SET(SUDODEM_EXEC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
SET(SUDODEM_PY_PATH ${SUDODEM_LIB_PATH}/py)
SET(SUDODEM_DOC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc/sudodem${SUFFIX})
SET(SUDODEM_MAN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR})

SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


#===========================================================
IF(ENABLE_GUI)
  file(TO_CMAKE_PATH "$ENV{Qt6_ROOT}" Qt6_ROOT_CMAKE)
  MESSAGE(STATUS "Qt6 Root: ${Qt6_ROOT_CMAKE}")
  ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/gui")
ENDIF(ENABLE_GUI)
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/py")
#===========================================================

IF(NOT EXISTS "${CMAKE_BINARY_DIR}/sudodem")
 #EXECUTE_PROCESS(COMMAND ln -s ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/sudodem)
 execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/sudodem
 )
ENDIF ()

FILE(GLOB QHULL_SRC_LIB "lib/qhull/*.c")

if(WIN32)
  ADD_LIBRARY(qhull STATIC ${QHULL_SRC_LIB})
ELSE(WIN32)
  ADD_LIBRARY(qhull SHARED ${QHULL_SRC_LIB})
ENDIF(WIN32)

SET_TARGET_PROPERTIES(qhull PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(qhull)

if(NOT WIN32)
  INSTALL(TARGETS qhull RUNTIME DESTINATION ${SUDODEM_LIB_PATH})
ENDIF()
#===========================================================
FILE(GLOB VORO_SRC_LIB "lib/voro++/voro++.cc")
ADD_LIBRARY(voro++ SHARED ${VORO_SRC_LIB})

if(WIN32)
  set_target_properties(voro++ PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  INSTALL(TARGETS voro++ RUNTIME DESTINATION ${SUDODEM_EXEC_PATH})
else(WIN32)
  INSTALL(TARGETS voro++ DESTINATION ${SUDODEM_LIB_PATH})
ENDIF(WIN32)

SET_TARGET_PROPERTIES(voro++ PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(voro++)
#===========================================================


FILE(GLOB SRC_CORE "core/*.cpp")
FILE(GLOB_RECURSE SRC_PKG "pkg/*.cpp")
FILE(GLOB SRC_LIB "lib/*.cpp")

SET(SRC_LIB "${SRC_LIB};lib/base/Math.cpp;lib/base/Singleton.cpp;lib/factory/ClassRegistryInstantiation.cpp")
SET(SRC_LIB "${SRC_LIB};lib/serialization/Serializable.cpp;lib/pyutil/gil.cpp;${GUI_SRC_LIB};${CGAL_SRC_LIB}")

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

if (WIN32)
  TARGET_COMPILE_DEFINITIONS(sudodem PRIVATE NOMINMAX SUDODEM_STATIC_MEMBER_ORIGIN)
  set_target_properties(sudodem PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  target_compile_definitions(sudodem PRIVATE SUDODEM_SINGLETON_EXPORT)
ENDIF()

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

#===========================================================


ADD_EXECUTABLE(sudodem2d ${CMAKE_CURRENT_SOURCE_DIR}/core/main/sudodem.cpp)

if(BUILD_STANDALONE_WIN)
  TARGET_COMPILE_DEFINITIONS(sudodem2d PRIVATE BUILD_STANDALONE_WIN)
endif()

if(BUILD_STANDALONE_LINUX)
  TARGET_COMPILE_DEFINITIONS(sudodem2d PRIVATE BUILD_STANDALONE_LINUX)
endif()

if(BUILD_STANDALONE_MACOS)
  TARGET_COMPILE_DEFINITIONS(sudodem2d PRIVATE BUILD_STANDALONE_MACOS)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
 set_target_properties(sudodem2d PROPERTIES AUTOMOC ON)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# Add libomp rpath when OpenMP is enabled
	if(OPENMP_FOUND)
		SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../lib/;@loader_path/../lib/sudodem;@loader_path/../lib/sudodem/py/sudodem;@loader_path/../lib/3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};${OpenMP_ROOT}/lib")
	else()
		SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../lib/;@loader_path/../lib/sudodem;@loader_path/../lib/sudodem/py/sudodem;@loader_path/../lib/3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
	endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/;$ORIGIN/../lib/sudodem;$ORIGIN/../lib/3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
else()
	SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/sudodem/py/sudodem;$ORIGIN/../lib/3rdlibs;")
endif()

ADD_LIBRARY(boot MODULE ${CMAKE_CURRENT_SOURCE_DIR}/core/main/pyboot.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}" LINK_FLAGS "-Wl,-z,origin,--as-needed" INSTALL_RPATH "$ORIGIN;$ORIGIN/../../;$ORIGIN/../../../;$ORIGIN/../../../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "" INSTALL_RPATH "@loader_path;@loader_path/../../;@loader_path/../../../;@loader_path/../../../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
elseif(WIN32)
	set_target_properties(boot PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX "${PYTHON_MODULE_EXTENSION}")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	TARGET_LINK_LIBRARIES(sudodem qhull ${Python_LIBRARIES} -lrt pybind11::module BZip2::BZip2 ZLIB::ZLIB ${SUDODEM_GLUT_TARGET} ${OPENGL_LIBRARIES})
else()
	# macOS and win
	if(OPENMP_FOUND)
		TARGET_LINK_LIBRARIES(sudodem qhull ${Python_LIBRARIES} pybind11::module BZip2::BZip2 ZLIB::ZLIB OpenMP::OpenMP_CXX ${SUDODEM_GLUT_TARGET} ${OPENGL_LIBRARIES})
	else()
		TARGET_LINK_LIBRARIES(sudodem qhull ${Python_LIBRARIES} pybind11::module BZip2::BZip2 ZLIB::ZLIB ${SUDODEM_GLUT_TARGET} ${OPENGL_LIBRARIES})
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET_TARGET_PROPERTIES(sudodem PROPERTIES LINK_FLAGS "-Wl,--as-needed" INSTALL_RPATH "$ORIGIN;$ORIGIN/../;$ORIGIN/../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};" )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# Add libomp rpath when OpenMP is enabled
	if(OPENMP_FOUND)
		SET_TARGET_PROPERTIES(sudodem PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../;@loader_path/../3rdlibs;@loader_path/../3rdlibs/py;@loader_path/py/sudodem/qt;@loader_path/../../../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};${OpenMP_ROOT}/lib;")
	else()
		SET_TARGET_PROPERTIES(sudodem PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../;@loader_path/../3rdlibs;@loader_path/../3rdlibs/py;@loader_path/py/sudodem/qt;@loader_path/../../../3rdlibs;${Python_RUNTIME_LIBRARY_DIRS};")
	endif()
endif()

TARGET_LINK_LIBRARIES(boot PRIVATE sudodem pybind11::module)
if(ENABLE_GUI)
 # Link boot against GLUT to resolve GLUT symbols needed by the Python package
 TARGET_LINK_LIBRARIES(boot PRIVATE ${SUDODEM_GLUT_TARGET})
endif() 

if(WIN32)
  TARGET_LINK_LIBRARIES(sudodem2d sudodem ${Python_LIBRARIES} pybind11::embed)
ELSE(WIN32)
  TARGET_LINK_LIBRARIES(sudodem2d sudodem ${Python_LIBRARIES} pybind11::embed pthread)
ENDIF(WIN32)

IF(ENABLE_GUI)
 TARGET_INCLUDE_DIRECTORIES(sudodem2d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gui)
 TARGET_LINK_LIBRARIES(sudodem2d _GLViewer_core ${GUI_LIBS})
 if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib/;$ORIGIN/../lib/sudodem;$ORIGIN/../lib/sudodem/py/sudodem/qt;$ORIGIN/../lib/3rdlibs;${Qt6_ROOT_CMAKE}/lib;${Python_RUNTIME_LIBRARY_DIRS};" )
 elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Add libomp rpath when OpenMP is enabled
  if(OPENMP_FOUND)
    SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../lib/;@loader_path/../lib/sudodem;@loader_path/../lib/sudodem/py/sudodem;@loader_path/../lib/sudodem/py/sudodem/qt;@loader_path/../lib/3rdlibs;${Qt6_ROOT_CMAKE}/lib;${Python_RUNTIME_LIBRARY_DIRS};${OpenMP_ROOT}/lib;")
  else()
    SET_TARGET_PROPERTIES(sudodem2d PROPERTIES INSTALL_RPATH "@loader_path;@loader_path/../lib/;@loader_path/../lib/sudodem;@loader_path/../lib/sudodem/py/sudodem;@loader_path/../lib/sudodem/py/sudodem/qt;@loader_path/../lib/3rdlibs;${Qt6_ROOT_CMAKE}/lib;${Python_RUNTIME_LIBRARY_DIRS};")
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

INSTALL(TARGETS boot DESTINATION "${SUDODEM_PY_PATH}/sudodem/")

if(WIN32)
  INSTALL(FILES "${CMAKE_SOURCE_DIR}/../thirdparty/freeglut_win64/bin/freeglut.dll" DESTINATION ${SUDODEM_EXEC_PATH})
  INSTALL(TARGETS sudodem RUNTIME DESTINATION ${SUDODEM_EXEC_PATH})
  INSTALL(TARGETS sudodem2d RUNTIME DESTINATION ${SUDODEM_EXEC_PATH}) 
else(WIN32)
  INSTALL(TARGETS sudodem DESTINATION ${SUDODEM_LIB_PATH})
  INSTALL(TARGETS sudodem2d DESTINATION ${SUDODEM_EXEC_PATH})
endif(WIN32)

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

get_target_property(_sudodem_links sudodem LINK_LIBRARIES)
message(STATUS "sudodem LINK_LIBRARIES = ${_sudodem_links}")

#===========================================================

if(WIN32)
  file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/../minieigen/lib" _minieigen_lib_dir)
  file(TO_NATIVE_PATH "${SUDODEM_PY_PATH}" _sudodem_py_path)
  add_custom_target(minieigen_install
      COMMENT "Copy minieigen if present"
      COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command
          "$files = Get-ChildItem -Path '${_minieigen_lib_dir}' -Filter 'minieigen*' -ErrorAction SilentlyContinue; if ($files) { Copy-Item -Path $files.FullName -Destination '${_sudodem_py_path}' -Force -ErrorAction SilentlyContinue }"
    VERBATIM
  )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_custom_target(minieigen_install
    COMMAND sh -c "set -- \"${CMAKE_SOURCE_DIR}/../minieigen/lib\"/minieigen*; echo first=$1; echo dst='${SUDODEM_PY_PATH}'; [ -e \"$1\" ] && mkdir -p \"${SUDODEM_PY_PATH}\" && cp -fv \"${CMAKE_SOURCE_DIR}/../minieigen/lib\"/minieigen* \"${SUDODEM_PY_PATH}/\" || true"
    VERBATIM
    USES_TERMINAL
)
endif()

#===========================================================

if(WIN32 AND BUILD_STANDALONE_WIN)

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)

  include(InstallRequiredSystemLibraries)
  set(PACK_WIN_RUNTIME_DLLS "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" CACHE INTERNAL "MSVC/OpenMP runtime DLLs to copy into the Windows package")

  set(PACK_WIN_VCRUNTIME_DLLS "")
  set(PACK_WIN_VCOMP_DLLS "")
  
  foreach(_dll IN LISTS PACK_WIN_RUNTIME_DLLS)
      get_filename_component(_dll_name "${_dll}" NAME)
      string(TOLOWER "${_dll_name}" _dll_name_lc)
      message(STATUS "Pack-VC RUNTIME DLLs: ${_dll_name_lc}")
      if(_dll_name_lc MATCHES "^vcruntime[0-9_]*\\.dll$")
          list(APPEND PACK_WIN_VCRUNTIME_DLLS "${_dll}")
      endif()
  endforeach()

  if(OpenMP_CXX_FOUND)
    if(DEFINED MSVC_OPENMP_DIR AND EXISTS "${MSVC_OPENMP_DIR}")
        file(GLOB PACK_WIN_OPENMP_DLLS "${MSVC_OPENMP_DIR}/vcomp*.dll")

        if(PACK_WIN_OPENMP_DLLS)
            message(STATUS "Pack-OpenMP DLL: ${PACK_WIN_OPENMP_DLLS}")
            foreach(_dll IN LISTS PACK_WIN_OPENMP_DLLS)
              if(EXISTS "${_dll}")
                  list(APPEND PACK_WIN_VCOMP_DLLS "${_dll}")
              else()
                  message(WARNING "OpenMP DLL listed by CMake does not exist: ${_dll}")
              endif()
            endforeach()
        else()
            message(WARNING "MSVC_OPENMP_DIR exists, but no vcomp*.dll was found in it: ${MSVC_OPENMP_DIR}")
        endif()
    else()
        message(WARNING "MSVC_OPENMP_DIR is not defined or does not exist.")
    endif()
  endif()

  set(_pack_win_copy_cmds)
  foreach(_dll IN LISTS PACK_WIN_VCRUNTIME_DLLS)
    if(EXISTS "${_dll}")
        list(APPEND _pack_win_copy_cmds
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "${SUDODEM_EXEC_PATH}")
    else()
        message(WARNING "Runtime DLL listed by CMake does not exist: ${_dll}")
    endif()
  endforeach()
  foreach(_dll IN LISTS PACK_WIN_VCOMP_DLLS)
    if(EXISTS "${_dll}")
        list(APPEND _pack_win_copy_cmds
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dll}" "${SUDODEM_EXEC_PATH}")
    else()
        message(WARNING "OpenMP DLL listed by CMake does not exist: ${_dll}")
    endif()
  endforeach()
   
  add_custom_target(pack_win
    COMMENT "Windows package -> Creating standalone program at install prefix: ${CMAKE_INSTALL_PREFIX}"

    # Step 1: qt pack
    COMMAND windeployqt --release --no-translations --dir "${SUDODEM_EXEC_PATH}" "${SUDODEM_EXEC_PATH}/sudodem2d.exe"

    # Step 2: Copy non-qt C++ DLLs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONDA_PREFIX_CMAKE}/Library/bin/zlib.dll" "${SUDODEM_EXEC_PATH}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CONDA_PREFIX_CMAKE}/Library/bin/libbz2.dll" "${SUDODEM_EXEC_PATH}/"

    # Step 3: MSVC runtime and OpenMP (from compiler redist)
    ${_pack_win_copy_cmds}

  )


#======================= LINUX PACK =======================#
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND BUILD_STANDALONE_LINUX)

  if(ENABLE_GUI AND Qt6_FOUND)

      qt_generate_deploy_script(
            TARGET sudodem2d
            OUTPUT_SCRIPT deploy_script
            CONTENT "

        include(\"\${QT_DEPLOY_SUPPORT}\")

        qt_deploy_runtime_dependencies(
            EXECUTABLE \"bin/sudodem2d\"
            BIN_DIR \"bin\"
            LIB_DIR \"lib/3rdlibs\"
            NO_TRANSLATIONS
        )
        "
        )
  endif()

  # non-Qt

  set(BZIP2_LIB_PATH "${BZIP2_LIBRARIES}")
  file(REAL_PATH "${BZIP2_LIB_PATH}" BZIP2_REAL_LIB_PATH)
  get_filename_component(BZIP2_REAL_LIB_NAME "${BZIP2_REAL_LIB_PATH}" NAME)

  set(ZLIB_LIB_PATH "${ZLIB_LIBRARIES}")
  file(REAL_PATH "${ZLIB_LIB_PATH}" ZLIB_REAL_LIB_PATH)
  get_filename_component(ZLIB_REAL_LIB_NAME "${ZLIB_REAL_LIB_PATH}" NAME)

  # OpenGL
  get_target_property(OPENGL_LIB_PATH OpenGL::OpenGL IMPORTED_LOCATION)
  get_target_property(GLU_LIB_PATH    OpenGL::GLU IMPORTED_LOCATION)
  get_target_property(GLUT_LIB_PATH   GLUT::GLUT IMPORTED_LOCATION)

  file(REAL_PATH "${OPENGL_LIB_PATH}" OPENGL_REAL_LIB_PATH)
  file(REAL_PATH "${GLU_LIB_PATH}"    GLU_REAL_LIB_PATH)
  file(REAL_PATH "${GLUT_LIB_PATH}"   GLUT_REAL_LIB_PATH)

  get_filename_component(OPENGL_REAL_LIB_NAME "${OPENGL_REAL_LIB_PATH}" NAME)
  get_filename_component(GLU_REAL_LIB_NAME    "${GLU_REAL_LIB_PATH}" NAME)
  get_filename_component(GLUT_REAL_LIB_NAME   "${GLUT_REAL_LIB_PATH}" NAME)

  # OpenMP
  set(OPENMP_REAL_LIBS "")
  foreach(_lib ${OpenMP_CXX_LIBRARIES})
    if(EXISTS "${_lib}")
      get_filename_component(_ext "${_lib}" EXT)
      if(_ext STREQUAL ".so" OR _lib MATCHES "\\.so\\.")
        file(REAL_PATH "${_lib}" _real_lib)
        list(APPEND OPENMP_REAL_LIBS "${_real_lib}")
        message(STATUS "Pack OpenMP runtime: ${_real_lib}")
      endif()
    endif()
  endforeach()
  list(REMOVE_DUPLICATES OPENMP_REAL_LIBS)

  set(PACK_3RDLIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/3rdlibs")

  add_custom_target(pack_linux
    COMMENT "Linux package -> standalone program at install prefix: ${CMAKE_INSTALL_PREFIX}"

    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/plugins/platforms"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PACK_3RDLIB_DIR}"

    # Qt: only if generated
    COMMAND ${CMAKE_COMMAND} -E echo "Running Linux pack target"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Qt6_ROOT_CMAKE}/plugins/platforms/libqxcb.so"
            "${CMAKE_INSTALL_PREFIX}/plugins/platforms/libqxcb.so"
    USES_TERMINAL
  )

  if(ENABLE_GUI AND Qt6_FOUND)
    add_custom_command(TARGET pack_linux POST_BUILD
      COMMAND ${CMAKE_COMMAND}
            -DQT_DEPLOY_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -P "${deploy_script}"
      USES_TERMINAL
    )
  endif()

  add_custom_command(TARGET pack_linux POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BZIP2_REAL_LIB_PATH}" "${PACK_3RDLIB_DIR}/${BZIP2_REAL_LIB_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ZLIB_REAL_LIB_PATH}" "${PACK_3RDLIB_DIR}/${ZLIB_REAL_LIB_NAME}"

    COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${BZIP2_REAL_LIB_NAME}" "${PACK_3RDLIB_DIR}/libbz2.so"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${ZLIB_REAL_LIB_NAME}" "${PACK_3RDLIB_DIR}/libz.so"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLU_REAL_LIB_PATH}" "${PACK_3RDLIB_DIR}/${GLU_REAL_LIB_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLUT_REAL_LIB_PATH}" "${PACK_3RDLIB_DIR}/${GLUT_REAL_LIB_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENGL_REAL_LIB_PATH}" "${PACK_3RDLIB_DIR}/${OPENGL_REAL_LIB_NAME}"
  )

  foreach(_lib IN LISTS OPENMP_REAL_LIBS)
    get_filename_component(_name "${_lib}" NAME)
    add_custom_command(TARGET pack_linux POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${_lib}" "${PACK_3RDLIB_DIR}/${_name}"
    )
  endforeach()

endif()
