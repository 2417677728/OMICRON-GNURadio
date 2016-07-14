# Install script for directory: /home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/python_tutorial" TYPE FILE FILES "/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/cmake/Modules/python_tutorialConfig.cmake")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/include/python_tutorial/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/lib/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/swig/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/python/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/grc/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/apps/cmake_install.cmake")
  INCLUDE("/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/docs/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/samuelvm/GNURadio-Tutorials/Tutorial3/gr-python_tutorial/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
