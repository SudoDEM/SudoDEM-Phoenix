IF(${ENABLE_GUI})
  SET(CMAKE_AUTOMOC ON)
  
  # Include directories
  INCLUDE_DIRECTORIES(${Python_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})  # For local headers
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/qglviewer)  # For QGLViewer headers
  
  # Add Qt6 include paths for compatibility header
  IF(QT_VERSION_MAJOR EQUAL 6)
    INCLUDE_DIRECTORIES(${Qt6Widgets_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS} ${Qt6OpenGL_INCLUDE_DIRS} ${Qt6Widgets_PRIVATE_INCLUDE_DIRS})
    # Add Qt6 framework path for includes
    INCLUDE_DIRECTORIES("/opt/homebrew/lib/QtOpenGLWidgets.framework/Headers")
    INCLUDE_DIRECTORIES("/opt/homebrew/lib/QtWidgets.framework/Headers")
    INCLUDE_DIRECTORIES("/opt/homebrew/lib/QtGui.framework/Headers")
    INCLUDE_DIRECTORIES("/opt/homebrew/lib/QtOpenGL.framework/Headers")
  ENDIF()
  
  # ============================================
  # Build libQGLViewer as static library
  # ============================================
  SET(QGLVIEWER_SOURCES
    qglviewer/camera.cpp
    qglviewer/constraint.cpp
    qglviewer/frame.cpp
    qglviewer/keyFrameInterpolator.cpp
    qglviewer/manipulatedCameraFrame.cpp
    qglviewer/manipulatedFrame.cpp
    qglviewer/mouseGrabber.cpp
    qglviewer/qglviewer.cpp
    qglviewer/quaternion.cpp
    qglviewer/saveSnapshot.cpp
    qglviewer/vec.cpp
    # VRender sources
    qglviewer/VRender/BackFaceCullingOptimizer.cpp
    qglviewer/VRender/BSPSortMethod.cpp
    qglviewer/VRender/EPSExporter.cpp
    qglviewer/VRender/Exporter.cpp
    qglviewer/VRender/FIGExporter.cpp
    qglviewer/VRender/gpc.cpp
    qglviewer/VRender/NVector3.cpp
    qglviewer/VRender/ParserGL.cpp
    qglviewer/VRender/Primitive.cpp
    qglviewer/VRender/PrimitivePositioning.cpp
    qglviewer/VRender/TopologicalSortMethod.cpp
    qglviewer/VRender/Vector2.cpp
    qglviewer/VRender/Vector3.cpp
    qglviewer/VRender/VisibilityOptimizer.cpp
    qglviewer/VRender/VRender.cpp
  )
  
  ADD_LIBRARY(qglviewer STATIC ${QGLVIEWER_SOURCES})
  
  IF(QT_VERSION_MAJOR EQUAL 6)
    # Qt6 provides all OpenGL functionality, no need for GLUT
    TARGET_LINK_LIBRARIES(qglviewer Qt6::Widgets Qt6::Gui Qt6::OpenGL Qt6::Xml Qt6::OpenGLWidgets ${OPENGL_LIBRARIES})
  ELSE()
    TARGET_LINK_LIBRARIES(qglviewer Qt5::Widgets Qt5::Xml Qt5::OpenGL ${GLUT_LIBRARY} ${OPENGL_LIBRARIES})
  ENDIF()
  
  MESSAGE(STATUS "Building libQGLViewer as static library from embedded sources")
  
  # ============================================
  # Build _GLViewer module
  # ============================================
  SET(_GLViewer_SOURCE_FILES 
    GLViewer.cpp
    _GLViewer.cpp
    OpenGLManager.cpp
    GLViewerDisplay.cpp
    GLViewerMouse.cpp
    Controller.cpp
    ControllerSimulation.cpp
    ControllerDisplay.cpp
    ControllerGenerate.cpp
    ControllerLayout.cpp
    ControllerSignals.cpp
    ControllerPython.cpp
    ControllerStyle.cpp
    SudoStyle.cpp
    ControllerResources.qrc
  )

  ADD_LIBRARY(_GLViewer SHARED ${_GLViewer_SOURCE_FILES})
  SET_TARGET_PROPERTIES(_GLViewer PROPERTIES PREFIX "" INSTALL_RPATH "$ORIGIN;$ORIGIN/../../../../3rdlibs;")

  IF(QT_VERSION_MAJOR EQUAL 6)
    # Qt6 provides all OpenGL functionality, no need for GLUT
    TARGET_LINK_LIBRARIES(_GLViewer Qt6::Widgets Qt6::Gui Qt6::OpenGL Qt6::Xml Qt6::OpenGLWidgets ${OPENGL_LIBRARIES} ${QT_LIBRARIES} sudodem qglviewer)
  ELSE()
    TARGET_LINK_LIBRARIES(_GLViewer Qt5::Widgets Qt5::Xml Qt5::OpenGL ${GLUT_LIBRARY} ${OPENGL_LIBRARIES} ${QT_LIBRARIES} sudodem qglviewer)
  ENDIF()

  IF(GL2PS_FOUND AND ENABLE_GL2PS)
    TARGET_LINK_LIBRARIES(_GLViewer ${GL2PS_LIBRARIES})
  ENDIF(GL2PS_FOUND AND ENABLE_GL2PS)

  INSTALL(TARGETS _GLViewer DESTINATION ${SUDODEM_PY_PATH}/sudodem/qt)
  
  # Install the static library (optional, for debugging)
  INSTALL(TARGETS qglviewer DESTINATION ${SUDODEM_LIB_PATH})

  FILE(GLOB filesPYQT "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
  INSTALL(FILES ${filesPYQT} DESTINATION ${SUDODEM_PY_PATH}/sudodem/qt)

  MESSAGE(STATUS "Python3 is used in the GUI")
  SET(ENV{PYTHONPATH} "${CMAKE_SOURCE_DIR}/../3rdlib/HeaderLib/lib/py")

  # Skip resource compilation for Qt6 if Qt6 tools are not available
  IF(QT_VERSION_MAJOR EQUAL 6)
    MESSAGE(STATUS "Qt6 detected - skipping pyrcc/pyuic resource compilation (Qt6 tools not available)")
  ELSE()
    EXECUTE_PROCESS(
              COMMAND "${CMAKE_SOURCE_DIR}/../SudoDEMTools/PyQt5/bin/pyrcc5" "-o" "${CMAKE_BINARY_DIR}/img_rc.py" "img.qrc"
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              RESULT_VARIABLE rv
              )

    EXECUTE_PROCESS(
              COMMAND "${CMAKE_SOURCE_DIR}/../SudoDEMTools/PyQt5/bin/pyuic5" "-o" "${CMAKE_BINARY_DIR}/ui_controller.py" "controller.ui"
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              RESULT_VARIABLE rv
              )

    INSTALL(FILES ${CMAKE_BINARY_DIR}/img_rc.py ${CMAKE_BINARY_DIR}/ui_controller.py  DESTINATION ${SUDODEM_PY_PATH}/sudodem/qt)
  ENDIF()
ENDIF(${ENABLE_GUI})