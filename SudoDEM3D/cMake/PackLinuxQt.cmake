cmake_minimum_required(VERSION 3.16)

# Inputs:
#   INSTALL_PREFIX
#   QT_ROOT   # directory containing Qt.
#
# Copies recursively discovered deps from QTLIB_ROOT into:
#   ${INSTALL_PREFIX}/lib/3rdlibs

set(QTLIB_ROOT "${QT_ROOT}/lib")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_PREFIX}/plugins/platforms"
)

if(DEFINED ENV{XDG_SESSION_TYPE} AND NOT "$ENV{XDG_SESSION_TYPE}" STREQUAL "")
    set(_session_type "$ENV{XDG_SESSION_TYPE}")
elseif(DEFINED ENV{WAYLAND_DISPLAY} AND NOT "$ENV{WAYLAND_DISPLAY}" STREQUAL "")
    set(_session_type "wayland")
elseif(DEFINED ENV{DISPLAY} AND NOT "$ENV{DISPLAY}" STREQUAL "")
    set(_session_type "x11")
else()
    set(_session_type "unknown")
endif()


if(_session_type STREQUAL "wayland")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_PREFIX}/plugins/wayland-decoration-client"
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_PREFIX}/plugins/wayland-graphics-integration-client"
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_PREFIX}/plugins/wayland-shell-integration"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/platforms/libqwayland-generic.so"
                "${INSTALL_PREFIX}/plugins/platforms/libqwayland-generic.so"
    )
    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/platforms/libqwayland-generic.so"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/wayland-decoration-client/libadwaita.so"
                "${INSTALL_PREFIX}/plugins/wayland-decoration-client/libadwaita.so"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/wayland-decoration-client/libbradient.so"
                "${INSTALL_PREFIX}/plugins/wayland-decoration-client/libbradient.so"
    )

    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/wayland-decoration-client/libadwaita.so"
    )
    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/wayland-decoration-client/libbradient.so"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"
                "${INSTALL_PREFIX}/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"
    )
    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/wayland-shell-integration/libxdg-shell.so"
                "${INSTALL_PREFIX}/plugins/wayland-shell-integration/libxdg-shell.so"
    )
    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/wayland-shell-integration/libxdg-shell.so"
    )

elseif(_session_type STREQUAL "x11")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_PREFIX}/plugins/xcbglintegrations"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/platforms/libqxcb.so"
                "${INSTALL_PREFIX}/plugins/platforms/libqxcb.so"
    )

    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/platforms/libqxcb.so"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_ROOT}/plugins/xcbglintegrations/libqxcb-glx-integration.so"
                "${INSTALL_PREFIX}/plugins/xcbglintegrations/libqxcb-glx-integration.so"
    )
    execute_process(
        COMMAND patchelf --add-rpath "\$ORIGIN/../../lib/3rdlibs" "${INSTALL_PREFIX}/plugins/xcbglintegrations/libqxcb-glx-integration.so"
    )
endif()



set(_qt_src_dir "${QTLIB_ROOT}")
set(_qt_dst_dir "${INSTALL_PREFIX}/lib/3rdlibs")
file(MAKE_DIRECTORY "${_qt_dst_dir}")

function(get_elf_needed_libs input_file output_var)
    if(NOT EXISTS "${input_file}")
        set(${output_var} "" PARENT_SCOPE)
        return()
    endif()

    execute_process(
        COMMAND readelf -d "${input_file}"
        RESULT_VARIABLE _ret
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT _ret EQUAL 0)
        message(WARNING "readelf failed for ${input_file}: ${_err}")
        set(${output_var} "" PARENT_SCOPE)
        return()
    endif()

    string(REPLACE "\n" ";" _lines "${_out}")

    set(_deps "")
    foreach(_line IN LISTS _lines)
        if(_line MATCHES "NEEDED")
            string(REGEX REPLACE ".*\\[([^][]+)\\].*" "\\1" _lib "${_line}")
            if(NOT _lib STREQUAL "${_line}")
                list(APPEND _deps "${_lib}")
            endif()
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _deps)
    set(${output_var} "${_deps}" PARENT_SCOPE)
endfunction()

# Return TRUE only if the soname can be sourced from QTLIB_ROOT.
function(is_copyable_from_qtroot soname out_var)
    if(EXISTS "${_qt_src_dir}/${soname}")
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        # fallback: allow matching real file/symlink variants if needed
        file(GLOB _cands
            "${_qt_src_dir}/${soname}"
            "${_qt_src_dir}/${soname}*"
        )
        list(LENGTH _cands _n)
        if(_n GREATER 0)
            set(${out_var} TRUE PARENT_SCOPE)
        else()
            set(${out_var} FALSE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Breadth-first recursive closure over anything that lives under QTLIB_ROOT.
function(resolve_copyable_dep_closure seed_libs output_var)
    set(_resolved "")
    set(_queue ${seed_libs})

    while(_queue)
        list(GET _queue 0 _cur)
        list(REMOVE_AT _queue 0)

        if(_cur IN_LIST _resolved)
            continue()
        endif()

        is_copyable_from_qtroot("${_cur}" _ok)
        if(NOT _ok)
            continue()
        endif()

        list(APPEND _resolved "${_cur}")

        set(_cur_path "${_qt_src_dir}/${_cur}")
        if(NOT EXISTS "${_cur_path}")
            # if exact soname path doesn't exist, pick the first match
            file(GLOB _matches "${_qt_src_dir}/${_cur}" "${_qt_src_dir}/${_cur}*")
            list(LENGTH _matches _mcount)
            if(_mcount GREATER 0)
                list(GET _matches 0 _cur_path)
            else()
                message(WARNING "Cannot open queued dep from QTLIB_ROOT: ${_cur}")
                continue()
            endif()
        endif()

        get_elf_needed_libs("${_cur_path}" _deps)

        foreach(_dep IN LISTS _deps)
            is_copyable_from_qtroot("${_dep}" _dep_ok)
            if(_dep_ok)
                if(NOT _dep IN_LIST _resolved AND NOT _dep IN_LIST _queue)
                    list(APPEND _queue "${_dep}")
                endif()
            endif()
        endforeach()
    endwhile()

    list(REMOVE_DUPLICATES _resolved)
    set(${output_var} "${_resolved}" PARENT_SCOPE)
endfunction()

# ---- seed from your installed binaries ----

set(_seed_files
    "${INSTALL_PREFIX}/bin/sudodem3d"
    "${INSTALL_PREFIX}/lib/sudodem/libsudodem.so"
)

file(GLOB_RECURSE _py_so_files
    "${INSTALL_PREFIX}/lib/sudodem/py/*.so"
)

file(GLOB_RECURSE _plugin_so_files
    "${INSTALL_PREFIX}/plugins/*.so"
)

list(APPEND _seed_files ${_py_so_files})
list(APPEND _seed_files ${_plugin_so_files})
list(REMOVE_DUPLICATES _seed_files)

set(_seed_sonames "")
foreach(_f IN LISTS _seed_files)
    get_elf_needed_libs("${_f}" _deps)
    foreach(_d IN LISTS _deps)
        is_copyable_from_qtroot("${_d}" _ok)
        if(_ok)
            list(APPEND _seed_sonames "${_d}")
        endif()
    endforeach()
endforeach()

list(REMOVE_DUPLICATES _seed_sonames)
message(STATUS "initial deps found in QTLIB_ROOT = ${_seed_sonames}")

# ---- recursive closure inside QTLIB_ROOT ----

resolve_copyable_dep_closure("${_seed_sonames}" _all_copyable_deps)
message(STATUS "full recursive deps from QTLIB_ROOT = ${_all_copyable_deps}")

# ---- copy ----

foreach(_lib IN LISTS _all_copyable_deps)
    set(_src "${_qt_src_dir}/${_lib}")

    if(EXISTS "${_src}")
        message(STATUS "copy: ${_src} -> ${_qt_dst_dir}")
        file(INSTALL
            DESTINATION "${_qt_dst_dir}"
            TYPE FILE
            FOLLOW_SYMLINK_CHAIN
            FILES "${_src}"
        )
    else()
        # fallback for variants
        file(GLOB _matches "${_qt_src_dir}/${_lib}" "${_qt_src_dir}/${_lib}*")
        list(LENGTH _matches _mcount)
        if(_mcount GREATER 0)
            list(GET _matches 0 _src2)
            message(STATUS "copy: ${_src2} -> ${_qt_dst_dir}")
            file(INSTALL
                DESTINATION "${_qt_dst_dir}"
                TYPE FILE
                FOLLOW_SYMLINK_CHAIN
                FILES "${_src2}"
            )
        else()
            message(WARNING "missing source under QTLIB_ROOT for ${_lib}")
        endif()
    endif()
endforeach()
