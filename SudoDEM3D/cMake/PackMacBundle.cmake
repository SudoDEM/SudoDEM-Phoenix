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

# SudoDEM3D src in CMAKE_PREFIX_INSTALL
set(SRC_BIN_DIR      "${INSTALL_PREFIX}/bin")
set(SRC_LIB_DIR      "${INSTALL_PREFIX}/lib")
set(SRC_3RDLIB_DIR   "${INSTALL_PREFIX}/lib/3rdlibs")
set(SRC_PROJECT_DIR  "${INSTALL_PREFIX}/lib/sudodem")
set(SRC_PY_DIR       "${INSTALL_PREFIX}/lib/sudodem/py")
set(SRC_EXE          "${SRC_BIN_DIR}/${APP_EXE_NAME}")

# Embed python (test on cpython 3.13)
set(USER_PYTHON_BIN_DIR "${USER_PYTHON_DIR}/bin")
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
set(APP_PYTHON_BIN_DIR "${APP_PYTHON_ROOT_DIR}/bin")
set(APP_PYTHON_STD_DIR "${APP_PYTHON_LIB_DIR}/python3.13")
set(APP_SUDODEM_PYTHON_ROOT_DIR "${APP_RESOURCES_DIR}/sudodempython")

# Maocs icon setup 
set(APP_ICON_SOURCE "${INSTALL_PREFIX}/share/doc/sudodem/img/sudodem-logo-note.png")
set(APP_ICON_NAME "SudoDEM3D")
set(APP_ICONSET_DIR "${INSTALL_PREFIX}/share/SudoDEM3D.iconset")                                                          
set(APP_ICON_DEST "${INSTALL_PREFIX}/share/${APP_ICON_NAME}.icns")

# Generate icon
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_ICONSET_DIR}")

execute_process(COMMAND sips -z 16 16     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_16x16.png" OUTPUT_QUIET)
execute_process(COMMAND sips -z 32 32     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_16x16@2x.png" OUTPUT_QUIET)      
execute_process(COMMAND sips -z 32 32     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_32x32.png" OUTPUT_QUIET)
execute_process(COMMAND sips -z 64 64     "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_32x32@2x.png" OUTPUT_QUIET)      
execute_process(COMMAND sips -z 128 128   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_128x128.png" OUTPUT_QUIET)
execute_process(COMMAND sips -z 256 256   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_128x128@2x.png" OUTPUT_QUIET)    
execute_process(COMMAND sips -z 256 256   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_256x256.png" OUTPUT_QUIET)
execute_process(COMMAND sips -z 512 512   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_256x256@2x.png" OUTPUT_QUIET)    
execute_process(COMMAND sips -z 512 512   "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_512x512.png" OUTPUT_QUIET)       
execute_process(COMMAND sips -z 1024 1024 "${APP_ICON_SOURCE}" --out "${APP_ICONSET_DIR}/icon_512x512@2x.png" OUTPUT_QUIET)
                                                                                                                
execute_process(COMMAND iconutil -c icns "${APP_ICONSET_DIR}" -o "${APP_ICON_DEST}")
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf "${APP_ICONSET_DIR}")


# Macos bundle creation
file(REMOVE_RECURSE "${APP_BUNDLE_DIR}")
file(MAKE_DIRECTORY "${APP_MACOS_DIR}")
file(MAKE_DIRECTORY "${APP_FRAME_DIR}")
file(MAKE_DIRECTORY "${APP_RESOURCES_DIR}")

file(MAKE_DIRECTORY "${APP_PYTHON_ROOT_DIR}")
file(MAKE_DIRECTORY "${APP_PYTHON_LIB_DIR}")
file(MAKE_DIRECTORY "${APP_PYTHON_BIN_DIR}")
file(MAKE_DIRECTORY "${APP_PYTHON_STD_DIR}")
file(MAKE_DIRECTORY "${APP_SUDODEM_PYTHON_ROOT_DIR}")

# 1 icon copy
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${APP_ICON_DEST}" "${APP_RESOURCES_DIR}/"
)

# 2 preprocess on SudoDEM3D: link symbol changed to @rpath

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
                    execute_process(
                        COMMAND install_name_tool -change "${old_path}" "@rpath/libpython3.13.dylib" "${target}"
                    )
                endif()
            elseif(old_name STREQUAL "libomp.dylib")
                if(NOT old_path STREQUAL "@rpath/libomp.dylib")
                    execute_process(
                        COMMAND install_name_tool -change "${old_path}" "@rpath/libomp.dylib" "${target}"
                    )
                endif()
            endif()
        endif()
    endforeach()
endforeach()

# 3 add rpath for SudoDEM3D python

file(GLOB _minieigen_file "${SRC_PY_DIR}/minieigen*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../Frameworks"  ${_minieigen_file})

file(GLOB _boot_file "${SRC_PY_DIR}/sudodem/boot*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../Frameworks"  ${_boot_file})

file(GLOB _wrapper_file "${SRC_PY_DIR}/sudodem/wrapper*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../Frameworks"  ${_wrapper_file})

file(GLOB _glviewer_file "${SRC_PY_DIR}/sudodem/qt/_GLViewer*")
execute_process(COMMAND install_name_tool  -add_rpath "@loader_path/../../../../Frameworks"  ${_glviewer_file})

# 4 copy SudoDEM3D C++
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SRC_EXE}" "${APP_EXE}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC_3RDLIB_DIR}" "${APP_FRAME_DIR}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/qhull.dylib" "${APP_FRAME_DIR}/qhull.dylib")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/libsudodem.dylib" "${APP_FRAME_DIR}/libsudodem.dylib")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/voro++.dylib" "${APP_FRAME_DIR}/voro++.dylib")
execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${SRC_LIB_DIR}/sudodem/py/sudodem/qt/lib_GLViewer_core.dylib" "${APP_FRAME_DIR}/lib_GLViewer_core.dylib")

# # 5 copy SudoDEM3D Python
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC_PY_DIR}" "${APP_SUDODEM_PYTHON_ROOT_DIR}")

# # 6 copy user python copy python
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${USER_PYTHON_STD_DIR}" "${APP_PYTHON_STD_DIR}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${USER_PYTHON_BIN_DIR}/python3.13" "${APP_PYTHON_BIN_DIR}/")
execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "python3.13" "${APP_PYTHON_BIN_DIR}/python3")

execute_process(COMMAND otool -L "${APP_PYTHON_BIN_DIR}/python3.13" OUTPUT_VARIABLE pybinout)

string(REGEX MATCH "(/[^\n\t ]*libpython3\\.13[^ \n\t]*\\.dylib|@rpath/libpython3\\.13[^ \n\t]*\\.dylib|@executable_path/[^\n\t ]*libpython3\\.13[^ \n\t]*\\.dylib|@loader_path/[^\n\t ]*libpython3\\.13[^ \n\t]*\\.dylib)"
pybinoldlib
"${pybinout}")

if(pybinoldlib)
    execute_process(COMMAND install_name_tool -change "${pybinoldlib}" "@rpath/libpython3.13.dylib" "${APP_PYTHON_BIN_DIR}/python3.13")
endif()
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../Frameworks" "${APP_PYTHON_BIN_DIR}/python3.13")

# # 7 copy python module deps lzma, cypto and sssl
set(PY_LIBDYNLOAD_DIR "${APP_PYTHON_STD_DIR}/lib-dynload")
set(PY_SO_RPATH "@loader_path/../../../../../Frameworks")

file(GLOB PY_SO_FILES "${PY_LIBDYNLOAD_DIR}/*.so")
set(ALL_DEPS "")
foreach(so IN LISTS PY_SO_FILES)
    execute_process(
        COMMAND otool -L "${so}"
        OUTPUT_VARIABLE out
    )
    string(REPLACE "\n" ";" lines "${out}")
    foreach(line IN LISTS lines)
        string(STRIP "${line}" line)
        if(line MATCHES "^([^ ]+)[ ]+\\(")
            set(dep "${CMAKE_MATCH_1}")

            if(dep MATCHES "^/System/" OR dep MATCHES "^/usr/lib/")
                continue()
            endif()

            if(dep MATCHES "^@rpath/" OR dep MATCHES "^@loader_path/" OR dep MATCHES "^@executable_path/")
                continue()
            endif()

            if(IS_ABSOLUTE "${dep}")
                list(APPEND ALL_DEPS "${dep}")
            endif()
        endif()
    endforeach()
endforeach()
list(REMOVE_DUPLICATES ALL_DEPS)

# message(STATUS "ALL_DEPS = ${ALL_DEPS}")

set(COPIED_REALS "")
set(COPIED_REAL_NAMES "")

foreach(dep IN LISTS ALL_DEPS)
    file(REAL_PATH "${dep}" real_dep)
    
    get_filename_component(real_name "${real_dep}" NAME)
    get_filename_component(dep_name "${dep}" NAME)

    file(COPY "${real_dep}" DESTINATION "${APP_FRAME_DIR}")
    execute_process(COMMAND chmod u+w "${APP_FRAME_DIR}/${real_name}")

    list(APPEND COPIED_REALS "${APP_FRAME_DIR}/${real_name}")
    list(APPEND COPIED_REAL_NAMES "${real_name}")

    execute_process(
        COMMAND install_name_tool -id "@rpath/${real_name}" "${APP_FRAME_DIR}/${real_name}"
    )

    set(cur "${dep}")
    while(IS_SYMLINK "${cur}")
        get_filename_component(link_name "${cur}" NAME)
        execute_process(
            COMMAND readlink "${cur}"
            OUTPUT_VARIABLE link_target
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        file(REMOVE "${APP_FRAME_DIR}/${link_name}")

        if(IS_ABSOLUTE "${link_target}")
            execute_process(
                COMMAND ln -s "${real_name}" "${APP_FRAME_DIR}/${link_name}"
            )
            file(REAL_PATH "${link_target}" cur)
        else()
            execute_process(
                COMMAND ln -s "${link_target}" "${APP_FRAME_DIR}/${link_name}"
            )
            get_filename_component(cur_dir "${cur}" DIRECTORY)
            set(cur "${cur_dir}/${link_target}")
        endif()
    endwhile()

    # if original basename differs from real basename, add one more symlink
    if(NOT dep_name STREQUAL real_name)
        file(REMOVE "${APP_FRAME_DIR}/${dep_name}")
        execute_process(
            COMMAND ln -s "${real_name}" "${APP_FRAME_DIR}/${dep_name}"
        )
    endif()
endforeach()

list(REMOVE_DUPLICATES COPIED_REALS)
list(REMOVE_DUPLICATES COPIED_REAL_NAMES)

# message(STATUS "COPIED_REALS = ${COPIED_REALS}")


foreach(lib IN LISTS COPIED_REALS)
    execute_process(
        COMMAND otool -L "${lib}" OUTPUT_VARIABLE out
    )
    string(REPLACE "\n" ";" lines "${out}")
    foreach(line IN LISTS lines)
        string(STRIP "${line}" line)
        if(line MATCHES "^([^ ]+)[ ]+\\(")
            set(dep "${CMAKE_MATCH_1}")

            if(dep MATCHES "^/System/" OR dep MATCHES "^/usr/lib/")
                continue()
            endif()
            if(dep MATCHES "^@rpath/" OR dep MATCHES "^@loader_path/" OR dep MATCHES "^@executable_path/")
                continue()
            endif()
            if(NOT IS_ABSOLUTE "${dep}")
                continue()
            endif()

            file(REAL_PATH "${dep}" dep_real)
            get_filename_component(dep_real_name "${dep_real}" NAME)

            list(FIND COPIED_REAL_NAMES "${dep_real_name}" found_idx)
            if(NOT found_idx EQUAL -1)
                execute_process(
                    COMMAND install_name_tool -change "${dep}" "@rpath/${dep_real_name}" "${lib}"
                )
            endif()
        endif()
    endforeach()
endforeach()


foreach(so IN LISTS PY_SO_FILES)
    execute_process(
        COMMAND otool -L "${so}" OUTPUT_VARIABLE out
    )
    set(this_so_changed_to_rpath FALSE)

    string(REPLACE "\n" ";" lines "${out}")
    foreach(line IN LISTS lines)
        string(STRIP "${line}" line)
        if(line MATCHES "^([^ ]+)[ ]+\\(")
            set(dep "${CMAKE_MATCH_1}")

            if(dep MATCHES "^/System/" OR dep MATCHES "^/usr/lib/")
                continue()
            endif()
            if(dep MATCHES "^@rpath/" OR dep MATCHES "^@loader_path/" OR dep MATCHES "^@executable_path/")
                continue()
            endif()
            if(NOT IS_ABSOLUTE "${dep}")
                continue()
            endif()

            file(REAL_PATH "${dep}" dep_real)
            get_filename_component(dep_real_name "${dep_real}" NAME)

            list(FIND COPIED_REAL_NAMES "${dep_real_name}" found_idx)

            if(NOT found_idx EQUAL -1)
                execute_process(
                    COMMAND install_name_tool -change "${dep}" "@rpath/${dep_real_name}" "${so}"
                )
                set(this_so_changed_to_rpath TRUE)

            endif()
        endif()
    endforeach()

    if(this_so_changed_to_rpath)
        execute_process(COMMAND otool -l "${so}" OUTPUT_VARIABLE lout ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

        set(existing_rpaths "")
        string(REPLACE "\n" ";" llines "${lout}")

        set(_expect_path FALSE)
        foreach(ll IN LISTS llines)
            string(STRIP "${ll}" ll)

            if(ll STREQUAL "cmd LC_RPATH")
                set(_expect_path TRUE)
                continue()
            endif()

            if(_expect_path AND ll MATCHES "^path (.+) \\(offset [0-9]+\\)$")
                list(APPEND existing_rpaths "${CMAKE_MATCH_1}")
                set(_expect_path FALSE)
            endif()
        endforeach()

        set(py_so_rpath_present FALSE)
        set(rpaths_to_delete "")

        foreach(rp IN LISTS existing_rpaths)
            if(rp STREQUAL "${PY_SO_RPATH}")
                set(py_so_rpath_present TRUE)
                continue()
            endif()

            if(rp MATCHES "^/System/" OR rp MATCHES "^/usr/lib/")
                continue()
            endif()
            if(rp MATCHES "^@loader_path/" OR rp MATCHES "^@executable_path/")
                continue()
            endif()

            set(_delete_this FALSE)

            if(IS_ABSOLUTE "${rp}")
                file(REAL_PATH "${rp}" rp_real)
            else()
                set(rp_real "")
            endif()

            foreach(dep_src IN LISTS ALL_DEPS)

                if(IS_ABSOLUTE "${dep_src}")
                    file(REAL_PATH "${dep_src}" dep_src_real)
                else()
                    set(dep_src_real "")
                endif()

                get_filename_component(dep_src_dir      "${dep_src}"      DIRECTORY)
                get_filename_component(dep_src_real_dir "${dep_src_real}" DIRECTORY)

                if(rp STREQUAL "${dep_src_dir}" OR rp STREQUAL "${dep_src_real_dir}" OR
                  (NOT rp_real STREQUAL "" AND rp_real STREQUAL "${dep_src_real_dir}"))
                    set(_delete_this TRUE)
                    break()
                endif()
            endforeach()

            if(_delete_this)
                list(APPEND rpaths_to_delete "${rp}")
            endif()
        endforeach()

        foreach(old_rp IN LISTS rpaths_to_delete)
            execute_process(
                COMMAND install_name_tool -delete_rpath "${old_rp}" "${so}"
                RESULT_VARIABLE _del_ret
                ERROR_VARIABLE  _del_err
                OUTPUT_QUIET
                ERROR_STRIP_TRAILING_WHITESPACE
            )
            if(_del_ret EQUAL 0)
                if(old_rp STREQUAL "${PY_SO_RPATH}")
                    set(py_so_rpath_present FALSE)
                endif()
            else()
                message(STATUS "skip delete_rpath for ${so}: ${_del_err}")
            endif()
        endforeach()

        if(NOT py_so_rpath_present)
            execute_process(
                COMMAND install_name_tool -add_rpath "${PY_SO_RPATH}" "${so}"
                RESULT_VARIABLE _add_ret
                ERROR_VARIABLE  _add_err
                OUTPUT_QUIET
                ERROR_STRIP_TRAILING_WHITESPACE
            )
            if(NOT _add_ret EQUAL 0)
                message(STATUS "skip add_rpath for ${so}: ${_add_err}")
            endif()
        endif()
    endif()
endforeach()


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

# # 10 pack Qt
execute_process(COMMAND chmod +x "${APP_EXE}" COMMAND_ECHO STDOUT)

execute_process(COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${APP_BUNDLE_DIR}" -verbose=2 COMMAND_ECHO STDOUT)

# # 11 sign
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
foreach(f IN LISTS _fw_dylibs)
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
    sign_one("${fw}")
endforeach()


file(GLOB_RECURSE _frameworks_py_native
    "${FRAMEWORKS_DIR}/py/*.so"
    "${FRAMEWORKS_DIR}/py/*.dylib"
)
list(LENGTH _frameworks_py_native _frameworks_py_native_count)
foreach(f IN LISTS _frameworks_py_native)
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
foreach(f IN LISTS _plugins_native)
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
foreach(f IN LISTS _resources_py_native)
    if(NOT IS_SYMLINK "${f}")
        sign_one("${f}")
    else()
        message(STATUS "skip resources/python symlink='${f}'")
    endif()
endforeach()


file(GLOB _macos_bins "${MACOS_DIR}/*")
list(LENGTH _macos_bins _macos_bins_count)
foreach(bin IN LISTS _macos_bins)
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
