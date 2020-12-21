# Copyright (C) 2017-2018 Jonathan Muller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(cppast VERSION 0.0)

# Allow custom cmake overrides
#set(CPPAST_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/module)
#set(CPPAST_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
#set(CMAKE_MODULE_PATH ${CPPAST_CMAKE_DIR} ${CMAKE_MODULE_PATH})

# Use ctest for dashboard submissions
# include(CTest)

# options
option(CPPAST_BUILD_TEST "whether or not to build the tests" OFF)
option(CPPAST_BUILD_EXAMPLE "whether or not to build the examples" OFF)
option(CPPAST_BUILD_TOOL "whether or not to build the tool" OFF)
#option(CPPAST_ENABLE_INSTALL "Generate the install target" OFF)


if(${CPPAST_BUILD_TEST} AND (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_test OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for the self integration test
else()
    set(build_test OFF)
endif()

if(${CPPAST_BUILD_EXAMPLE} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_example OFF)
else()
    set(build_example OFF)
endif()

if(${CPPAST_BUILD_TOOL} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_tool OFF)
else()
    set(build_tool OFF)
endif()

# 
set(targets_export_name cppast-targets)
set(CPPAST_CMAKE_DIR "lib/cmake/cppast" CACHE STRING
    "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")

include(external/external.cmake)

if(build_test AND CPPAST_TEST_GCOV AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
endif()

add_subdirectory(src)

if(${build_test})
    enable_testing()
    add_subdirectory(test)
endif()
if(${build_example})
    add_subdirectory(example)
endif()
if(${build_tool})
    add_subdirectory(tool)
endif()

### Install setting ###
#if(CPPAST_ENABLE_INSTALL)
  ### ---[ Create the config.h file
  set(cppast_config_h_in "${CMAKE_CURRENT_SOURCE_DIR}/cppast_config.h.in")
  set(cppast_config_h "${CMAKE_CURRENT_BINARY_DIR}/include/cppast/cppast_config.h")
  configure_file("${cppast_config_h_in}" "${cppast_config_h}")

  ### ---[ Add the includes subdirectories
  install(FILES "${cppast_config_h}" DESTINATION include)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include" DESTINATION .)

  ### ---[ Configure cppast-config.cmake
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  # Set up our directory structure for output libraries and binaries
  # (Note: these are the build locations, not the install locations)
  if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CPPAST_BINARY_DIR}/bin")
  endif()
  if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    if(UNIX)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CPPAST_BINARY_DIR}/lib")
    else()
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CPPAST_BINARY_DIR}/bin")
    endif()
  endif()
  if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CPPAST_BINARY_DIR}/lib")
  endif()

  set(version_config "${PROJECT_BINARY_DIR}/cppast-config-version.cmake")
  set(project_config "${PROJECT_BINARY_DIR}/cppast-config.cmake")

  # not use install(EXPORT)
  # use ***-config.cmake.in
  # set(INCLUDE_INSTALL_DIR include/ ... CACHE )
  # set(LIB_INSTALL_DIR lib/ ... CACHE )
  # set(SYSCONFIG_INSTALL_DIR etc/foo/ ... CACHE )
  # Generate the version, config and target files into the build directory.
  write_basic_package_version_file(
      ${version_config}
      VERSION ${VERSION}
      COMPATIBILITY AnyNewerVersion)
  configure_package_config_file(
      ${PROJECT_SOURCE_DIR}/cppast-config.cmake.in
      ${project_config}
      INSTALL_DESTINATION ${CPPAST_CMAKE_DIR})
      # PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
  #???(cppast::[libraryname])
  # export(TARGETS cppast NAMESPACE cppast::
  #        FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

  install(
      FILES ${project_config} ${version_config}
      DESTINATION ${CPPAST_CMAKE_DIR})

  # Install the export set for use with the install-tree
  install(EXPORT ${targets_export_name} 
          DESTINATION ${CPPAST_CMAKE_DIR}
          FILE ${targets_export_name}.cmake
          NAMESPACE cppast::)
  # install(EXPORT ${targets_export_name}
  #         DESTINATION ${CPPAST_CMAKE_DIR}
  #         NAMESPACE cppast::)

  # Install the header file and export the target
  # install(TARGETS cppast EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
  # library(not generate cppast-targets)
  # install(TARGETS cppast _cppast_tiny_process _cppast_libclang EXPORT ${targets_export_name}
  #                                  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/Debug
  #                                  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/Debug
  #                                  CONFIGURATIONS Debug)
  # install(TARGETS cppast _cppast_tiny_process _cppast_libclang EXPORT ${targets_export_name}
  #                                  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/Release
  #                                  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/Release
  #                                  CONFIGURATIONS Release)
  # internal use modules(set EXCLUDE_FROM_ALL)
  # If they are both build by the same CMakeLists.txt you can simply call
  # https://stackoverflow.com/questions/5378528/install-export-problem-for-shared-lib-with-dependencies
  # install(TARGETS cppast _cppast_tiny_process _cppast_libclang EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
  # src/CMakeLists.txt
  # install(TARGETS cppast EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
#endif()
