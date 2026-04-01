########################################################################
# - Try to find libfobos-sdr-agile
# Once done this will define
#  LIBFOBOS_SDR_AGILE_FOUND - System has libfobos-sdr-agile
#  LIBFOBOS_SDR_AGILE_INCLUDE_DIRS - The libfobos-sdr-agile include directories
#  LIBFOBOS_SDR_AGILE_LIBRARIES - The libfobos-sdr-agile libraries
#  LIBFOBOS_SDR_AGILE_LIB_DIRS - The libfobos-sdr-agile library directories
# (C) V.T.
# LGPL-2.1+
# 01.04.2026
########################################################################
set(CMAKE_GENERATOR_PLATFORM  x64)
#set(CMAKE_GENERATOR_PLATFORM  Win32)
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FOBOS_PKG libfobos_sdr)
    find_path(LIBFOBOS_SDR_AGILE_INCLUDE_DIRS
            NAMES fobos_sdr.h
            HINTS ${FOBOS_PKG_INCLUDE_DIRS} $ENV{FOBOS_DIR}/include
            PATHS /usr/local/include /usr/include /opt/include /opt/local/include)
    find_library(LIBFOBOS_SDR_AGILE_LIBRARIES
            NAMES fobos_sdr
            HINTS ${FOBOS_PKG_LIBRARY_DIRS} $ENV{FOBOS_DIR}/include
            PATHS /usr/local/lib /usr/lib /opt/lib /opt/local/lib)

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(LIBFOBOS_SDR_AGILE  DEFAULT_MSG
    LIBFOBOS_SDR_AGILE_LIBRARIES LIBFOBOS_SDR_AGILE_INCLUDE_DIRS)

    mark_as_advanced(LIBFOBOS_SDR_AGILE_INCLUDE_DIRS LIBFOBOS_SDR_AGILE_LIBRARIES)
else()
    if (CMAKE_GENERATOR_PLATFORM STREQUAL Win32)
        set(LIBFOBOS_SDR_AGILE_LIB_DIRS ${CMAKE_SOURCE_DIR}/fobos_sdr/x86)
        set(LIBFOBOS_SDR_AGILE_FOUND ON)
    endif()
    if (CMAKE_GENERATOR_PLATFORM STREQUAL x64)
        set(LIBFOBOS_SDR_AGILE_LIB_DIRS ${CMAKE_SOURCE_DIR}/fobos_sdr/x64)
        set(LIBFOBOS_SDR_AGILE_FOUND ON)
    endif()
    set(LIBFOBOS_SDR_AGILE_LIBRARIES ${LIBFOBOS_LIB_DIRS}/fobos_sdr.lib)
    set(LIBFOBOS_SDR_AGILE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/fobos_sdr/include)
endif()

message(>> "LIBFOBOS_SDR_AGILE_INCLUDE_DIRS: " ${LIBFOBOS_SDR_AGILE_INCLUDE_DIRS})
message(>> "LIBFOBOS_SDR_AGILE_LIBRARIES:    " ${LIBFOBOS_SDR_AGILE_LIBRARIES})
message(>> "LIBFOBOS_SDR_AGILE_LIB_DIRS:     " ${LIBFOBOS_SDR_AGILE_LIB_DIRS})
