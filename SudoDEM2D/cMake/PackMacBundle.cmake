cmake_minimum_required(VERSION 3.16)

#Inputs:
#INSTALL_PREFIX
#APP_NAME
#APP_EXE_NAME
#MACDEPLOYQT_EXECUTABLE

# cpython3 deps
# lib-dynload/_lzma.cpython-313-darwin.so:
#      /opt/homebrew/opt/xz/lib/liblzma.5.dylib
# lib-dynload/_hashlib.cpython-313-darwin.so:
#      /opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib
# lib-dynload/_ssl.cpython-313-darwin.so:
#      /opt/homebrew/opt/openssl@3/lib/libssl.3.dylib
#      /opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib

# SudoDEM2D src in CMAKE_PREFIX_INSTALL
set(SRC_BIN_DIR      "${INSTALL_PREFIX}/bin")
set(SRC_LIB_DIR      "${INSTALL_PREFIX}/lib")
set(SRC_3RDLIB_DIR   "${INSTALL_PREFIX}/lib/3rdlibs")
set(SRC_PROJECT_DIR  "${INSTALL_PREFIX}/lib/sudodem")
set(SRC_PY_DIR       "${INSTALL_PREFIX}/lib/sudodem/py")
set(SRC_EXE          "${SRC_BIN_DIR}/${APP_EXE_NAME}")

# Embed python (test on cpython 3.13)
set(USER_PYTHON_LIB_DIR "${USER_PYTHON_DIR}/lib")
set(USER_PYTHON_STD_DIR "${USER_PYTHON_LIB_DIR}/python3.13")

# Macos app bundle layout
# Store executable in Macos
# 3rd and builtin deps in Frameworks
# Embed python and SudoDEM python in Resources

set(APP_BUNDLE_DIR      "${INSTALL_PREFIX}/${APP_NAME}.app")
set(APP_CONTENTS_DIR    "${APP_BUNDLE_DIR}/Contents")
set(APP_FRAME_DIR       "${APP_CONTENTS_DIR}/Frameworks")
set(APP_MACOS_DIR       "${APP_CONTENTS_DIR}/MacOS")
set(APP_RESOURCES_DIR   "${APP_CONTENTS_DIR}/Resources")
set(APP_EXE             "${APP_MACOS_DIR}/${APP_EXE_NAME}")
set(APP_PLIST           "${APP_CONTENTS_DIR}/Info.plist")

set(APP_PYTHON_ROOT_DIR "${APP_RESOURCES_DIR}/python")
set(APP_PYTHON_LIB_DIR "${APP_PYTHON_ROOT_DIR}/lib")
set(APP_PYTHON_STD_DIR "${APP_PYTHON_LIB_DIR}/python3.13")
set(APP_SUDODEM_PYTHON_ROOT_DIR "${APP_RESOURCES_DIR}/sudodempython")

# Maocs icon setup 
set(APP_ICON_SOURCE "${INSTALL_PREFIX}/share/doc/sudodem/img/sudodem-logo-note.png")
set(APP_ICON_NAME "SudoDEM2D")
set(APP_ICONSET_DIR "${INSTALL_PREFIX}/share/SudoDEM2D.iconset")                                                          
set(APP_ICON_DEST "${INSTALL_PREFIX}/share/${APP_ICON_NAME}.icns")

# Generate icon
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_ICONSET_DIR}")

execute_process(COMMAND sips -z 16 16     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_16x16.png")
execute_process(COMMAND sips -z 32 32     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_16x16@2x.png")      
execute_process(COMMAND sips -z 32 32     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_32x32.png")
execute_process(COMMAND sips -z 64 64     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_32x32@2x.png")      
execute_process(COMMAND sips -z 128 128   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_128x128.png")
execute_process(COMMAND sips -z 256 256   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_128x128@2x.png")    
execute_process(COMMAND sips -z 256 256   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_256x256.png")
execute_process(COMMAND sips -z 512 512   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_256x256@2x.png")    
execute_process(COMMAND sips -z 512 512   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_512x512.png")       
execute_process(COMMAND sips -z 1024 1024 "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_512x512@2x.png")
                                                                                                                
execute_process(COMMAND iconutil -c icns "${APP_ICONSET_DIR}" -o "${APP_ICON_DEST}")
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf "${APP_ICONSET_DIR}")


# Macos bundle creation
file(REMOVE_RECURSE "${APP_BUNDLE_DIR}")
file(MAKE_DIRECTORY "${APP_MACOS_DIR}")
file(MAKE_DIRECTORY "${APP_FRAME_DIR}")
file(MAKE_DIRECTORY "${APP_RESOURCES_DIR}")

file(MAKE_DIRECTORY "${APP_PYTHON_ROOT_DIR}")
file(MAKE_DIRECTORY "${APP_PYTHON_LIB_DIR}")
file(MAKE_DIRECTORY "${APP_PYTHON_STD_DIR}")
file(MAKE_DIRECTORY "${APP_SUDODEM_PYTHON_ROOT_DIR}")

# 1 icon copy
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${APP_ICON_DEST}" "${APP_RESOURCES_DIR}/"
    COMMAND_ECHO STDOUT
)

# 2 preprocess on SudoDEM2D: link symbol changed to @rpath

set(PROJECT_BINARIES "${SRC_EXE}")
file(GLOB_RECURSE EXTRA_BINS
    "${SRC_LIB_DIR}/*.dylib"
    "${SRC_LIB_DIR}/*.so"
)
list(APPEND PROJECT_BINARIES ${EXTRA_BINS})
list(REMOVE_DUPLICATES PROJECT_BINARIES)

foreach(target IN LISTS PROJECT_BINARIES)
    execute_process(
        COMMAND otool -L "${target}"
        OUTPUT_VARIABLE OTOOL_OUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ECHO STDOUT
    )

    string(REPLACE "\n" ";" lines "${OTOOL_OUT}")

    foreach(line IN LISTS lines)
        string(STRIP "${line}" line)

        if(line STREQUAL "")
            continue()
        endif()

        if(line MATCHES ":$")
            continue()
        endif()

        if(line MATCHES "^([^ ]+)")
            set(old_path "${CMAKE_MATCH_1}")
            get_filename_component(old_name "${old_path}" NAME)

            if(old_name STREQUAL "libpython3.13.dylib")
                if(NOT old_path STREQUAL "@rpath/libpython3.13.dylib")
                    message(STATUS "Patch ${target}: ${old_path} -> @rpath/libpython3.13.dylib")
                    execute_process(
                        COMMAND install_name_tool -change "${old_path}" "@rpath/libpython3.13.dylib" "${target}"
                        COMMAND_ECHO STDOUT
                    )
                endif()
            elseif(old_name STREQUAL "libomp.dylib")
                if(NOT old_path STREQUAL "@rpath/libomp.dylib")
                    message(STATUS "Patch ${target}: ${old_path} -> @rpath/libomp.dylib")
                    execute_process(
                        COMMAND install_name_tool -change "${old_path}" "@rpath/libomp.dylib" "${target}"
                        COMMAND_ECHO STDOUT
                    )
                endif()
            endif()
        endif()
    endforeach()
endforeach()

# 3 add rpath for SudoDEM2D python

file(GLOB _minieigen_file "${SRC_PY_DIR}/minieigen*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../Frameworks"  ${_minieigen_file})

file(GLOB _boot_file "${SRC_PY_DIR}/sudodem/boot*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../Frameworks"  ${_boot_file})

file(GLOB _wrapper_file "${SRC_PY_DIR}/sudodem/wrapper*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../Frameworks"  ${_wrapper_file})

file(GLOB _glviewer_file "${SRC_PY_DIR}/sudodem/qt/_GLViewer*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../../Frameworks"  ${_glviewer_file})

# 4 copy SudoDEM2D C++
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SRC_EXE}" "${APP_EXE}"  COMMAND_ECHO STDOUT)

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC_3RDLIB_DIR}" "${APP_FRAME_DIR}" COMMAND_ECHO STDOUT)

execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/qhull.dylib" "${APP_FRAME_DIR}/qhull.dylib" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/libsudodem.dylib" "${APP_FRAME_DIR}/libsudodem.dylib" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/voro++.dylib" "${APP_FRAME_DIR}/voro++.dylib" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/py/sudodem/qt/lib_GLViewer_core.dylib" "${APP_FRAME_DIR}/lib_GLViewer_core.dylib" COMMAND_ECHO STDOUT)

# 5 copy SudoDEM2D Python
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC_PY_DIR}" "${APP_SUDODEM_PYTHON_ROOT_DIR}" COMMAND_ECHO STDOUT)

# 6 copy user python copy python
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${USER_PYTHON_STD_DIR}" "${APP_PYTHON_STD_DIR}" COMMAND_ECHO STDOUT)

# 7 copy python module deps lzma, cypto and sssl
set(LZMA_REAL_LIB_PATH "/opt/homebrew/opt/xz/lib/liblzma.5.dylib")
file(REAL_PATH "${LZMA_REAL_LIB_PATH}" LZMA_REAL_RESOLVED)
set(LZMA_REAL_LIB_NAME "liblzma.5.dylib")

set(CRYPTO_REAL_LIB_PATH "/opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib")
file(REAL_PATH "${CRYPTO_REAL_LIB_PATH}" CRYPTO_REAL_RESOLVED)
set(CRYPTO_REAL_LIB_NAME "libcrypto.3.dylib")

set(SSL_REAL_LIB_PATH "/opt/homebrew/opt/openssl@3/lib/libssl.3.dylib")
file(REAL_PATH "${SSL_REAL_LIB_PATH}" SSL_REAL_RESOLVED)
set(SSL_REAL_LIB_NAME "libssl.3.dylib")

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${LZMA_REAL_RESOLVED}" "${APP_FRAME_DIR}/${LZMA_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${CRYPTO_REAL_RESOLVED}" "${APP_FRAME_DIR}/${CRYPTO_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${SSL_REAL_RESOLVED}" "${APP_FRAME_DIR}/${SSL_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)

execute_process(COMMAND chmod u+w "${APP_FRAME_DIR}/${LZMA_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND chmod u+w "${APP_FRAME_DIR}/${CRYPTO_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND chmod u+w "${APP_FRAME_DIR}/${SSL_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)

execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${LZMA_REAL_LIB_NAME}" "${APP_FRAME_DIR}/liblzma.dylib" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${CRYPTO_REAL_LIB_NAME}" "${APP_FRAME_DIR}/libcrypto.dylib" COMMAND_ECHO STDOUT)
execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${SSL_REAL_LIB_NAME}" "${APP_FRAME_DIR}/libssl.dylib" COMMAND_ECHO STDOUT)

execute_process(COMMAND install_name_tool -id "@rpath/${LZMA_REAL_LIB_NAME}" "${APP_FRAME_DIR}/${LZMA_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND install_name_tool -id "@rpath/${CRYPTO_REAL_LIB_NAME}" "${APP_FRAME_DIR}/${CRYPTO_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)
execute_process(COMMAND install_name_tool -id "@rpath/${SSL_REAL_LIB_NAME}" "${APP_FRAME_DIR}/${SSL_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)

execute_process(COMMAND install_name_tool -change "${CRYPTO_REAL_RESOLVED}" "@rpath/libcrypto.3.dylib" "${APP_FRAME_DIR}/${SSL_REAL_LIB_NAME}" COMMAND_ECHO STDOUT)

# 8 python module link path modification
execute_process(
    COMMAND install_name_tool -change /opt/homebrew/opt/xz/lib/liblzma.5.dylib
    @rpath/liblzma.5.dylib
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_lzma.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)
execute_process(
    COMMAND install_name_tool -add_rpath
    @loader_path/../../../../../Frameworks
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_lzma.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)

execute_process(
    COMMAND install_name_tool -change /opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib
    @rpath/libcrypto.3.dylib
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_hashlib.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)
execute_process(
    COMMAND install_name_tool -add_rpath
    @loader_path/../../../../../Frameworks
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_hashlib.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)

execute_process(
    COMMAND install_name_tool -change /opt/homebrew/opt/openssl@3/lib/libssl.3.dylib
    @rpath/libssl.3.dylib
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_ssl.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)
execute_process(
    COMMAND install_name_tool -change /opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib
    @rpath/libcrypto.3.dylib
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_ssl.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)
execute_process(
    COMMAND install_name_tool -add_rpath
    @loader_path/../../../../../Frameworks
    "${APP_RESOURCES_DIR}/python/lib/python3.13/lib-dynload/_ssl.cpython-313-darwin.so"
    COMMAND_ECHO STDOUT
)

# 9 App Info.plist
file(WRITE "${APP_PLIST}" "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
        <!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
        <plist version=\"1.0\">
        <dict>
            <key>CFBundleDevelopmentRegion</key>
            <string>English</string>
            <key>CFBundleExecutable</key>
            <string>${APP_EXE_NAME}</string>
            <key>CFBundleIdentifier</key>
            <string>SudoSimLab.${APP_NAME}</string>
            <key>CFBundleIconFile</key>
            <string>${APP_ICON_NAME}</string>
            <key>CFBundleInfoDictionaryVersion</key>
            <string>6.0</string>
            <key>CFBundleName</key>
            <string>${APP_NAME}</string>
            <key>CFBundlePackageType</key>
            <string>APPL</string>
            <key>CFBundleShortVersionString</key>
            <string>1.0</string>
            <key>CFBundleVersion</key>
            <string>1.0</string>
        </dict>
        </plist>
        ")

# 10 pack Qt
execute_process(COMMAND chmod +x "${APP_EXE}" COMMAND_ECHO STDOUT)

execute_process(COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${APP_BUNDLE_DIR}" -verbose=2 COMMAND_ECHO STDOUT)

# 11 sign
if(NOT DEFINED SIGN_IDENTITY)
    set(SIGN_IDENTITY "-")
endif()

message(STATUS "===== SIGN DEBUG BEGIN =====")
message(STATUS "APP_BUNDLE='${APP_BUNDLE_DIR}'")
message(STATUS "SIGN_IDENTITY='${SIGN_IDENTITY}'")

set(FRAMEWORKS_DIR "${APP_BUNDLE_DIR}/Contents/Frameworks")
set(PLUGINS_DIR    "${APP_BUNDLE_DIR}/Contents/PlugIns")
set(RES_PY_DIR     "${APP_BUNDLE_DIR}/Contents/Resources/python")
set(MACOS_DIR      "${APP_BUNDLE_DIR}/Contents/MacOS")

message(STATUS "FRAMEWORKS_DIR='${FRAMEWORKS_DIR}'")
message(STATUS "PLUGINS_DIR='${PLUGINS_DIR}'")
message(STATUS "RES_PY_DIR='${RES_PY_DIR}'")
message(STATUS "MACOS_DIR='${MACOS_DIR}'")

if(EXISTS "${APP_BUNDLE_DIR}")
    message(STATUS "APP_BUNDLE_DIR exists")
else()
    message(FATAL_ERROR "APP_BUNDLE_DIR does not exist: ${APP_BUNDLE_DIR}")
endif()

if(EXISTS "${FRAMEWORKS_DIR}")
    message(STATUS "FRAMEWORKS_DIR exists")
else()
    message(WARNING "FRAMEWORKS_DIR does not exist: ${FRAMEWORKS_DIR}")
endif()

if(EXISTS "${PLUGINS_DIR}")
    message(STATUS "PLUGINS_DIR exists")
else()
    message(WARNING "PLUGINS_DIR does not exist: ${PLUGINS_DIR}")
endif()

if(EXISTS "${RES_PY_DIR}")
    message(STATUS "RES_PY_DIR exists")
else()
    message(WARNING "RES_PY_DIR does not exist: ${RES_PY_DIR}")
endif()

if(EXISTS "${MACOS_DIR}")
    message(STATUS "MACOS_DIR exists")
else()
    message(WARNING "MACOS_DIR does not exist: ${MACOS_DIR}")
endif()

find_program(CODESIGN_EXECUTABLE codesign REQUIRED)
message(STATUS "CODESIGN_EXECUTABLE='${CODESIGN_EXECUTABLE}'")

function(sign_one path)
    if(EXISTS "${path}")
        if(IS_SYMLINK "${path}")
            return()
        endif()

        execute_process(
            COMMAND "${CODESIGN_EXECUTABLE}" --force --sign "${SIGN_IDENTITY}" "${path}"
        )
    else()
        message(STATUS "skip missing: ${path}")
    endif()
endfunction()


file(GLOB _fw_dylibs "${FRAMEWORKS_DIR}/*.dylib")
list(LENGTH _fw_dylibs _fw_dylibs_count)
message(STATUS "Framework dylib count=${_fw_dylibs_count}")
foreach(f IN LISTS _fw_dylibs)
    message(STATUS "candidate framework dylib='${f}'")
    if(NOT IS_SYMLINK "${f}")
        sign_one("${f}")
    else()
        message(STATUS "skip framework dylib symlink='${f}'")
    endif()
endforeach()



file(GLOB _framework_bundles "${FRAMEWORKS_DIR}/*.framework")
list(LENGTH _framework_bundles _framework_bundles_count)
message(STATUS "Framework bundle count=${_framework_bundles_count}")
foreach(fw IN LISTS _framework_bundles)
    message(STATUS "candidate framework bundle='${fw}'")
    sign_one("${fw}")
endforeach()



file(GLOB_RECURSE _frameworks_py_native
    "${FRAMEWORKS_DIR}/py/*.so"
    "${FRAMEWORKS_DIR}/py/*.dylib"
)
list(LENGTH _frameworks_py_native _frameworks_py_native_count)
message(STATUS "Frameworks/py native count=${_frameworks_py_native_count}")
foreach(f IN LISTS _frameworks_py_native)
    message(STATUS "candidate frameworks/py native='${f}'")
    if(NOT IS_SYMLINK "${f}")
        sign_one("${f}")
    else()
        message(STATUS "skip frameworks/py symlink='${f}'")
    endif()
endforeach()



file(GLOB_RECURSE _plugins_native
    "${PLUGINS_DIR}/*.so"
    "${PLUGINS_DIR}/*.dylib"
)
list(LENGTH _plugins_native _plugins_native_count)
message(STATUS "PlugIns native count=${_plugins_native_count}")
foreach(f IN LISTS _plugins_native)
    message(STATUS "candidate plugin native='${f}'")
    if(NOT IS_SYMLINK "${f}")
        sign_one("${f}")
    else()
        message(STATUS "skip plugin symlink='${f}'")
    endif()
endforeach()



file(GLOB_RECURSE _resources_py_native
    "${RES_PY_DIR}/*.so"
    "${RES_PY_DIR}/*.dylib"
)
list(LENGTH _resources_py_native _resources_py_native_count)
message(STATUS "Resources/python native count=${_resources_py_native_count}")
foreach(f IN LISTS _resources_py_native)
    message(STATUS "candidate resources/python native='${f}'")
    if(NOT IS_SYMLINK "${f}")
        sign_one("${f}")
    else()
        message(STATUS "skip resources/python symlink='${f}'")
    endif()
endforeach()


file(GLOB _macos_bins "${MACOS_DIR}/*")
list(LENGTH _macos_bins _macos_bins_count)
message(STATUS "MacOS entry count=${_macos_bins_count}")
foreach(bin IN LISTS _macos_bins)
    message(STATUS "candidate MacOS entry='${bin}'")
    if(NOT IS_DIRECTORY "${bin}" AND NOT IS_SYMLINK "${bin}")
        sign_one("${bin}")
    else()
        message(STATUS "skip MacOS non-file or symlink='${bin}'")
    endif()
endforeach()


sign_one("${APP_BUNDLE_DIR}")

execute_process(
    COMMAND "${CODESIGN_EXECUTABLE}" --verify --deep --strict --verbose=4 "${APP_BUNDLE_DIR}"
    RESULT_VARIABLE _verify_ret
    OUTPUT_VARIABLE _verify_out
    ERROR_VARIABLE  _verify_err
)

if(NOT _verify_ret EQUAL 0)
    message(FATAL_ERROR "codesign verify failed")
endif()

message(STATUS "==== Final otool -L for main executable ====")
execute_process(
    COMMAND otool -L "${APP_EXE}"
    COMMAND_ECHO STDOUT
)