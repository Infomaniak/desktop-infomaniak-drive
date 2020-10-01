add_library("${synclib_NAME}_vfs_wincfapi" SHARED
    vfs_win.cpp
)

find_library(CLOUDFILEPROVIDER_LIBRARY CloudFileProvider HINTS ../CloudFileProvider/x64/Release)

target_link_libraries("${synclib_NAME}_vfs_wincfapi"
    ${synclib_NAME}
    ${CLOUDFILEPROVIDER_LIBRARY}
)

set_target_properties("${synclib_NAME}_vfs_wincfapi" PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    PREFIX ""
)

set(vfs_installdir "${PLUGINDIR}")

INSTALL(TARGETS "${synclib_NAME}_vfs_wincfapi"
  LIBRARY DESTINATION "${vfs_installdir}"
  RUNTIME DESTINATION "${vfs_installdir}"
)

