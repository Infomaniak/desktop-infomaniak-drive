project(libcommon)

set(libcommon_SRCS
    commonutility.cpp
)

add_library(${commonlib_NAME} STATIC ${libcommon_SRCS})
target_link_libraries(${commonlib_NAME} Qt5::Core)

GENERATE_EXPORT_HEADER(${commonlib_NAME}
    BASE_NAME ${commonlib_NAME}
    EXPORT_MACRO_NAME COMMON_EXPORT
    EXPORT_FILE_NAME commonlib.h
)

target_include_directories(${commonlib_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(${commonlib_NAME} PROPERTIES
    VERSION ${MIRALL_VERSION}
    SOVERSION ${MIRALL_SOVERSION}
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY}
)
