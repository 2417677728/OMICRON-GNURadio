INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ADAPTATIVEOFDM adaptativeOFDM)

FIND_PATH(
    ADAPTATIVEOFDM_INCLUDE_DIRS
    NAMES adaptativeOFDM/api.h
    HINTS $ENV{ADAPTATIVEOFDM_DIR}/include
        ${PC_ADAPTATIVEOFDM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ADAPTATIVEOFDM_LIBRARIES
    NAMES gnuradio-adaptativeOFDM
    HINTS $ENV{ADAPTATIVEOFDM_DIR}/lib
        ${PC_ADAPTATIVEOFDM_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ADAPTATIVEOFDM DEFAULT_MSG ADAPTATIVEOFDM_LIBRARIES ADAPTATIVEOFDM_INCLUDE_DIRS)
MARK_AS_ADVANCED(ADAPTATIVEOFDM_LIBRARIES ADAPTATIVEOFDM_INCLUDE_DIRS)
