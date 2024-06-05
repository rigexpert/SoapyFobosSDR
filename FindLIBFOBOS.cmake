########################################################################
# - Try to find libfobos
# Once done this will define
#  LIBFOBOS_FOUND - System has libfobos
#  LIBFOBOS_INCLUDE_DIRS - The libfobos include directories
#  LIBFOBOS_LIBRARIES - The libfobos libraries
#  LIBFOBOS_LIB_DIRS - The libfobos library directories
# (C) V.T.
# LGPL-2.1+
# 05.06.2024
########################################################################
set(CMAKE_GENERATOR_PLATFORM  x64)
#set(CMAKE_GENERATOR_PLATFORM  Win32)
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FOBOS_PKG libfobos)
    find_path(LIBFOBOS_INCLUDE_DIRS
            NAMES fobos.h
            HINTS ${FOBOS_PKG_INCLUDE_DIRS} $ENV{FOBOS_DIR}/include
            PATHS /usr/local/include /usr/include /opt/include /opt/local/include)
    find_library(LIBFOBOS_LIBRARIES
            NAMES fobos
            HINTS ${FOBOS_PKG_LIBRARY_DIRS} $ENV{FOBOS_DIR}/include
            PATHS /usr/local/lib /usr/lib /opt/lib /opt/local/lib)

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(LIBFOBOS  DEFAULT_MSG
    LIBFOBOS_LIBRARIES LIBFOBOS_INCLUDE_DIRS)

    mark_as_advanced(LIBFOBOS_INCLUDE_DIRS LIBFOBOS_LIBRARIES)
else()
    if (CMAKE_GENERATOR_PLATFORM STREQUAL Win32)
        set(LIBFOBOS_LIB_DIRS ${CMAKE_SOURCE_DIR}/fobos/x86)
        set(LIBFOBOS_FOUND ON)
    endif()
    if (CMAKE_GENERATOR_PLATFORM STREQUAL x64)
        set(LIBFOBOS_LIB_DIRS ${CMAKE_SOURCE_DIR}/fobos/x64)
        set(LIBFOBOS_FOUND ON)
    endif()
    set(LIBFOBOS_LIBRARIES ${LIBFOBOS_LIB_DIRS}/fobos.lib)
    set(LIBFOBOS_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/fobos/include)
endif()

message(>> "LIBFOBOS_INCLUDE_DIRS: " ${LIBFOBOS_INCLUDE_DIRS})
message(>> "LIBFOBOS_LIBRARIES:    " ${LIBFOBOS_LIBRARIES})
message(>> "LIBFOBOS_LIB_DIRS:     " ${LIBFOBOS_LIB_DIRS})
