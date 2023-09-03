cmake_minimum_required(VERSION 3.10)
project(mbfc)

# C/C++ version
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Platform config

set(ENABLE_ASAN 1)
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

set(WARNING_FLAGS
        "-Wall -Wextra \
        -Wshadow -Wundef -Wmaybe-uninitialized -Wno-deprecated-declarations \
        -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
        -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
        -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
        -Wempty-body -Wshift-negative-value \
        -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith -Wno-cast-function-type")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

# Address Sanitizer
if (ENABLE_ASAN)
        set(ASAN_FLAGS "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
        add_definitions(${ASAN_FLAGS})
        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS})
endif()

# Include
include_directories(
        ${PROJECT_SOURCE_DIR}
)

# Sources
file(GLOB mbfc_SOURCES "mbfc.c")

## ALL SOURCES
set(SOURCES
        ${mbfc_SOURCES}
)

# Executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(mbfc main.c ${SOURCES} ${INCLUDES})

# Link
target_link_libraries(
        mbfc PRIVATE
)

set_target_properties(
        mbfc
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build"
)
