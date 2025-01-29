cmake_minimum_required(VERSION 3.10)

# Define the project name and specify the required C++ standard
project(UfWebCameraV4L)

set(PROJECT_VERSION 1.0)

# Set the C++ standard for the project (you can change this if needed)
set(CMAKE_CXX_STANDARD 20)

# Specify the version (optional)
set(UF_NAME ${CMAKE_PROJECT_NAME})
set(EXE_NAME "${CMAKE_PROJECT_NAME}_TEST")

set(HEAD_DIR "./headers")
set(SRC_DIR "./sources")

set(SRC_CORE
    ./sources/webcamera_manager.cpp
)

option(LOG_DEBUG "Enable logging for debugging" OFF)

# Specify the source directory
set(AVEXECUTOR_DIR "/etc/avexecutor/")
message(STATUS "AVExecutor path: ${AVEXECUTOR_DIR}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_GENERATOR_PLATFORM MATCHES "AMD64")
	message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR} platform")
	set(AVL_LITE_LIB_DIR "${AVEXECUTOR_DIR}lib/x86_64-linux-gnu/")

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86 " OR CMAKE_GENERATOR_PLATFORM MATCHES "x86")
	message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR} platform")
	set(AVL_LITE_LIB_DIR "${AVEXECUTOR_DIR}lib/x86_64-linux-gnu//")

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
	message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR} platform")
	set(AVL_LITE_LIB_DIR "${AVEXECUTOR_DIR}lib/aarch64-linux-gnu/")
else()
	message(WARNING "Unsuported platform: ${CMAKE_SYSTEM_PROCESSOR}")
	set(AVL_LITE_LIB_DIR "${AVEXECUTOR_DIR}lib/x86_64-linux-gnu/")
endif()

set(AVL_LITE_LIB_DIR "${AVEXECUTOR_DIR}lib/x86_64-linux-gnu/")
set(AVL_LITE_INCLUDE_DIR "${AVEXECUTOR_DIR}include/")

set(SRC_PLATFORM
    ./sources/paraqueue.cpp
    ./sources/webcamera.cpp
)

set(SRC_FILES ${SRC_CORE} ${SRC_PLATFORM})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify the include directories for avexecutor
include_directories(${AVL_LITE_INCLUDE_DIR})

# Include directories for headers
include_directories(${HEAD_DIR})

# Specify the library directories for avexecutor and other dependencies
link_directories(${AVL_LITE_LIB_DIR})

add_subdirectory("./libv4l2cpp")

# Add the shared library target
add_library(${UF_NAME} SHARED sources/main.cpp ${SRC_FILES})

if(LOG_DEBUG)
    message(STATUS "LOG_DEBUG is set to ON")
    target_compile_definitions(${UF_NAME} PRIVATE LOG_DEBUG)
endif()

# set_property(TARGET ${UF_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})

# Link the necessary libraries
target_link_libraries(${UF_NAME} PUBLIC
    AVL_Lite
    UserFilters
    libv4l2cpp
)

# Set the output name for the shared library
set_target_properties(${UF_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Additional flags for compiling and linking (this is added automatically by CMake when using SHARED)
set_target_properties(${UF_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add a custom target for cleaning up
add_custom_target(clear
    COMMENT "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/CMakeFiles"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/CMakeCache.txt"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/Makefile"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/*.so"
    COMMENT "Removing CMake-generated files and the executable"

    COMMENT "${CMAKE_BINARY_DIR}/libv4l2cpp"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/libv4l2cpp/CMakeFiles"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/CMakeCache.txt"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/cmake_install.cmake"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/Makefile"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/*.so"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/*.a"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/libv4l2cpp/libv4l2cpptest"
    COMMENT "Removing CMake-generated files and the executable"
)
