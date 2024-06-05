########################################################################
# Soapy SDR support module for FobosSDR Devices
# (C) V.T.
# LGPL-2.1+
# 05.06.2024
########################################################################
cmake_minimum_required(VERSION 2.8.12)
project(SoapyFobosSDR CXX)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
########################################################################
# Searching for Soapy SDR development files
########################################################################
find_package(SoapySDR "0.4.0" NO_MODULE REQUIRED)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()
########################################################################
# Searching for FobosSDR ALI library
########################################################################
find_package(LIBFOBOS)
if (NOT LIBFOBOS_FOUND)
message(FATAL_ERROR "Fobos SDR development files not found...")
endif ()
########################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${LIBFOBOS_INCLUDE_DIRS})
########################################################################
if (APPLE)
    message(STATUS "Not tested yet")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wc++11-extensions")
endif(APPLE)
########################################################################
# Soapy+Fobos
########################################################################
SOAPY_SDR_MODULE_UTIL(
    TARGET FobosSDRSupport
    SOURCES
        SoapyFobosSDR.hpp
        Registration.cpp
        Settings.cpp
        Streaming.cpp
    LIBRARIES
       ${LIBFOBOS_LIBRARIES}
)
target_link_libraries(FobosSDRSupport PRIVATE ${LIBFOBOS_LIBRARIES})
########################################################################
# uninstall target
########################################################################
add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
########################################################################

