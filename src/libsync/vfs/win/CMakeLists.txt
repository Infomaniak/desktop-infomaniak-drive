add_library("${synclib_NAME}_vfs_win" SHARED
    vfs_win.cpp
)

set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
find_library(VFS_STATIC_LIBRARY NAMES vfs PATHS ${VFS_DIRECTORY} NO_DEFAULT_PATH)

target_link_libraries("${synclib_NAME}_vfs_win"
    ${synclib_NAME}
    ${VFS_STATIC_LIBRARY}
)

set_target_properties("${synclib_NAME}_vfs_win" PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
    PREFIX ""
)


