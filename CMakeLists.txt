cmake_minimum_required(VERSION 3.10.1)

project(MealyMachine)

SET(CMAKE_CXX_STANDARD 14)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(MAJOR_VERSION 1)
set(MINOR_VERSION 0)
set(REVISION_VERSION 0)

include(GenerateExportHeader)

set(LIB_NAME MealyMachine)

set(SOURCES
  src/${LIB_NAME}/MealyMachine.cpp
  )

set(INCLUDES
  src/${LIB_NAME}/Fwd.h
  src/${LIB_NAME}/MapTransitionChooser.h
  src/${LIB_NAME}/MealyMachine.h
  src/${LIB_NAME}/TransitionChooser.h
  )

add_library(${LIB_NAME} SHARED ${SOURCES} ${INCLUDES})
add_library(${LIB_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

option(${LIB_NAME}_BUILD_TESTS "toggle building of unit tests")
if(${LIB_NAME}_BUILD_TESTS)
  add_subdirectory(tests)
endif()


set(LIB_NAME_LOWER)
string(TOLOWER ${LIB_NAME} LIB_NAME_LOWER)

generate_export_header(${LIB_NAME} EXPORT_FILE_NAME ${LIB_NAME}/${LIB_NAME_LOWER}_export.h)

set(${LIB_NAME}_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${REVISION_VERSION})

set_property(TARGET ${LIB_NAME} PROPERTY VERSION ${${LIB_NAME}_VERSION})
set_property(TARGET ${LIB_NAME} PROPERTY SOVERSION 1)
set_property(TARGET ${LIB_NAME} PROPERTY INTERFACE_${LIB_NAME}_MAJOR_VERSION 1)
set_property(TARGET ${LIB_NAME} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING ${LIB_NAME}_MAJOR_VERSION)

install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

install(
  FILES
  ${INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME_LOWER}_export.h
  DESTINATION
  include/${LIB_NAME}
  COMPONENT
  Devel
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME}ConfigVersion.cmake
  VERSION ${${LIB_NAME}_VERSION}
  COMPATIBILITY SameMajorVersion
  )

export(EXPORT ${LIB_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME}Targets.cmake
  NAMESPACE ${LIB_NAME}::
  )

set(ConfigPackageLocation lib/cmake/${LIB_NAME})
install(EXPORT ${LIB_NAME}Targets
  FILE
  ${LIB_NAME}Targets.cmake
  NAMESPACE
  ${LIB_NAME}::
  DESTINATION
  ${ConfigPackageLocation}
  )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME}Config.cmake
  "include($" "{CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}Targets.cmake)\n"
  )

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}/${LIB_NAME}ConfigVersion.cmake
  DESTINATION
  ${ConfigPackageLocation}
  COMPONENT
  Devel
  )

