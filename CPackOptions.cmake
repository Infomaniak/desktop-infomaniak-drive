# This file is configured at cmake time, and loaded at cpack time.
# To pass variables to cpack from cmake, they must be configured
# in this file.

if(CPACK_GENERATOR MATCHES "NSIS")
    SET( CPACK_PACKAGE_ICON  C:/Projects/kdrive/admin/win/nsi/installer.ico ) #  A branding image that will be displayed on the top bar inside the installer.    installer.bmp
    SET( CPACK_NSIS_COMPRESSOR   "/SOLID lzma" ) # The arguments that will be passed to the NSIS SetCompressor command.    /SOLID lzma
endif(CPACK_GENERATOR MATCHES "NSIS")

set( CMAKE_SOURCE_DIR  C:/Projects/kdrive )
set( CMAKE_BINARY_DIR  C:/Projects/kdrive )
set( OEM_THEME_DIR      )

if ( DEFINED OEM_THEME_DIR AND EXISTS ${OEM_THEME_DIR}/OEM.cmake )
     include ( ${OEM_THEME_DIR}/OEM.cmake )
else ()
     include ( "${CMAKE_SOURCE_DIR}/OWNCLOUD.cmake" )
endif()

set( CRASHREPORTER_EXECUTABLE owncloud_crash_reporter)

set( BUILD_OWNCLOUD_OSX_BUNDLE OFF)
if(APPLE AND NOT BUILD_OWNCLOUD_OSX_BUNDLE)
    message( FATAL_ERROR "You're trying to build a bundle although you haven't built the client in bundle mode.\n Add -DBUILD_OWNCLOUD_OSX_BUNDLE=ON")
endif()

