add_library("${synclib_NAME}_vfs_win" SHARED
    vfs_win.cpp
)

set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
find_library(CLOUDFILEPROVIDER_STATIC_LIBRARY NAMES CloudFileProvider PATHS ${CLOUDFILEPROVIDER_DIRECTORY} NO_DEFAULT_PATH)

target_link_libraries("${synclib_NAME}_vfs_win"
    ${synclib_NAME}
    ${CLOUDFILEPROVIDER_STATIC_LIBRARY}
)

set_target_properties("${synclib_NAME}_vfs_win" PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    PREFIX ""
)

set(vfs_installdir "${PLUGINDIR}")

INSTALL(TARGETS "${synclib_NAME}_vfs_win"
    LIBRARY DESTINATION ${vfs_installdir}
    RUNTIME DESTINATION ${vfs_installdir}
)

set(CMAKE_FIND_LIBRARY_SUFFIXES .dll)
find_library(CLOUDFILEPROVIDER_SHARED_LIBRARY NAMES CloudFileProvider PATHS ${CLOUDFILEPROVIDER_DIRECTORY} NO_DEFAULT_PATH)

INSTALL(FILES ${CLOUDFILEPROVIDER_SHARED_LIBRARY}
    DESTINATION ${vfs_installdir}
)

message("-----")
message(${vfs_installdir})
message("-----")
