# Copyright (C) 2017-2018 Jonathan Muller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.8)
project(cppast VERSION 0.0)

# options
option(CPPAST_BUILD_TEST "whether or not to build the tests" OFF)
option(CPPAST_BUILD_EXAMPLE "whether or not to build the examples" OFF)
option(CPPAST_BUILD_TOOL "whether or not to build the tool" OFF)
#option(CPPAST_ENABLE_INSTALL "Generate the install target" OFF)


if(${CPPAST_BUILD_TEST} AND (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_test ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for the self integration test
else()
    set(build_test OFF)
endif()

if(${CPPAST_BUILD_EXAMPLE} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_example ON)
else()
    set(build_example OFF)
endif()

if(${CPPAST_BUILD_TOOL} OR (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    set(build_tool ON)
else()
    set(build_tool OFF)
endif()

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

  ### ---[ Add the libraries subdirectories
  #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/Debug/cppast.lib DESTINATION lib CONFIGURATIONS Debug)
  #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/Release/cppast.lib DESTINATION lib CONFIGURATIONS Release)
  #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/_cppast_tiny_process.lib DESTINATION lib CONFIGURATIONS Debug)
  #install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/_cppast_tiny_process.lib DESTINATION lib CONFIGURATIONS Release)

  ### ---[ Configure CPPASTConfig.cmake
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)
  set(CPPAST_CMAKE_DIR "lib/cmake/cppast" CACHE STRING
      "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
  set(version_config "${PROJECT_BINARY_DIR}/cppast-config-version.cmake")
  set(project_config "${PROJECT_BINARY_DIR}/cppast-config.cmake")
  set(targets_export_name cppast-targets)
  #set(targets_export_name2 _cppast_tiny_process-targets)

  # Generate the version, config and target files into the build directory.
  write_basic_package_version_file(
      ${version_config}
      VERSION ${VERSION}
      COMPATIBILITY AnyNewerVersion)
  configure_package_config_file(
      ${PROJECT_SOURCE_DIR}/cppast-config.cmake.in
      ${project_config}
      INSTALL_DESTINATION ${CPPAST_CMAKE_DIR})

  #???(cppast::[libraryname])
  export(TARGETS cppast NAMESPACE cppast::
         FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)
  #export(TARGETS tiny-process-library NAMESPACE tiny-process-library::
  #       FILE ${PROJECT_BINARY_DIR}/${targets_export_name2}.cmake)

  find_package(type_safe QUIET)
  if(type_safe_FOUND)
      #install(TARGETS cppast _cppast_tiny_process EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
      install(TARGETS cppast _cppast_tiny_process EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Debug CONFIGURATIONS Debug)
      install(TARGETS cppast _cppast_tiny_process EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Release CONFIGURATIONS Release)
  else()
      # use local
      # debug_assert/type_safe cppast/external module use
      find_package(debug_assert QUIET)
      if(debug_assert_FOUND)
          #install(TARGETS cppast _cppast_tiny_process type_safe EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
          install(TARGETS cppast _cppast_tiny_process type_safe EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Debug CONFIGURATIONS Debug)
          install(TARGETS cppast _cppast_tiny_process type_safe EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Release CONFIGURATIONS Release)
      else()
          #install(TARGETS cppast _cppast_tiny_process type_safe debug_assert EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
          install(TARGETS cppast _cppast_tiny_process type_safe debug_assert EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Debug CONFIGURATIONS Debug)
          install(TARGETS cppast _cppast_tiny_process type_safe debug_assert EXPORT ${targets_export_name} DESTINATION ${CMAKE_INSTALL_LIBDIR}/Release CONFIGURATIONS Release)
          install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/external/type_safe/external/debug_assert/debug_assert.hpp" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
      endif()
  endif()

  # Install version, config and target files.
  install(FILES ${project_config} ${version_config} DESTINATION ${CPPAST_CMAKE_DIR})
  #install(FILES ${project_config} ${version_config} DESTINATION ${CPPAST_CMAKE_DIR}/Debug CONFIGURATIONS Debug)
  #install(FILES ${project_config} ${version_config} DESTINATION ${CPPAST_CMAKE_DIR}/Release CONFIGURATIONS Release)
  export(TARGETS cppast NAMESPACE cppast:: FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)
  #install(EXPORT ${targets_export_name2} DESTINATION ${CPPAST_CMAKE_DIR})
  #export(TARGETS tiny-process-library NAMESPACE tiny-process-library:: FILE ${PROJECT_BINARY_DIR}/${targets_export_name2}.cmake)
#endif()
