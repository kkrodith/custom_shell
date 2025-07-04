cmake_minimum_required(VERSION 3.10)
project(UnifiedShell VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Platform detection
if(WIN32)
    set(PLATFORM "Windows")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM "Linux")
elseif(APPLE)
    set(PLATFORM "macOS")
endif()

message(STATUS "Building for platform: ${PLATFORM}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Find required packages
find_package(PkgConfig)

# Try to find readline using different methods
find_path(READLINE_INCLUDE_DIR readline/readline.h
    HINTS /usr/include /usr/local/include /opt/local/include
    PATH_SUFFIXES readline
)

find_library(READLINE_LIBRARY
    NAMES readline
    HINTS /usr/lib /usr/local/lib /opt/local/lib
    PATH_SUFFIXES x86_64-linux-gnu
)

find_library(HISTORY_LIBRARY
    NAMES history
    HINTS /usr/lib /usr/local/lib /opt/local/lib
    PATH_SUFFIXES x86_64-linux-gnu
)

# Check if readline was found
if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    message(STATUS "Found readline: ${READLINE_LIBRARY}")
    set(READLINE_FOUND TRUE)
else()
    message(WARNING "Readline not found. The shell will have limited functionality.")
    set(READLINE_FOUND FALSE)
endif()

# Source files
set(SOURCES
    main.cpp
    unified_shell.cpp
)

set(HEADERS
    unified_shell.hpp
)

# Create the executable
add_executable(unified_shell ${SOURCES} ${HEADERS})

# Set target properties
set_target_properties(unified_shell PROPERTIES
    OUTPUT_NAME "unified_shell"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Include directories
if(READLINE_FOUND)
    target_include_directories(unified_shell PRIVATE ${READLINE_INCLUDE_DIR})
    target_compile_definitions(unified_shell PRIVATE HAVE_READLINE=1)
endif()

# Link libraries
if(READLINE_FOUND)
    target_link_libraries(unified_shell ${READLINE_LIBRARY})
    if(HISTORY_LIBRARY)
        target_link_libraries(unified_shell ${HISTORY_LIBRARY})
    endif()
endif()

# Platform-specific linking
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(unified_shell ws2_32)
else()
    # Unix-like systems
    find_package(Threads REQUIRED)
    target_link_libraries(unified_shell Threads::Threads)
endif()

# Compiler-specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(unified_shell PRIVATE DEBUG=1)
    if(NOT MSVC)
        target_compile_options(unified_shell PRIVATE -g -O0)
    endif()
else()
    target_compile_definitions(unified_shell PRIVATE NDEBUG=1)
    if(NOT MSVC)
        target_compile_options(unified_shell PRIVATE -O3)
    endif()
endif()

# Installation settings
include(GNUInstallDirs)

install(TARGETS unified_shell
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Runtime
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "UnifiedShell")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform command execution environment")
set(CPACK_PACKAGE_DESCRIPTION "A unified shell that can execute both Windows and Linux commands on any platform")
set(CPACK_PACKAGE_CONTACT "developer@unifiedshell.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Unified Shell")
    set(CPACK_NSIS_PACKAGE_NAME "UnifiedShell")
    set(CPACK_NSIS_CONTACT "developer@unifiedshell.com")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libreadline8 | libreadline7 | libreadline6")
    set(CPACK_RPM_PACKAGE_REQUIRES "readline")
endif()

include(CPack)

# Custom targets for development
add_custom_target(check-deps
    COMMAND ${CMAKE_COMMAND} -E echo "Checking dependencies..."
    COMMAND ${CMAKE_COMMAND} -E echo "CMake version: ${CMAKE_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Platform: ${PLATFORM}"
    COMMAND ${CMAKE_COMMAND} -E echo "Readline found: ${READLINE_FOUND}"
    VERBATIM
)

add_custom_target(setup-dev
    COMMAND ${CMAKE_COMMAND} -E echo "Setting up development environment..."
    COMMAND ${CMAKE_COMMAND} -E echo "This target should install development dependencies"
    VERBATIM
)

# Test configuration
enable_testing()

add_test(NAME version_test
    COMMAND unified_shell --version
)

add_test(NAME help_test
    COMMAND unified_shell --help
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Readline support: ${READLINE_FOUND}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")