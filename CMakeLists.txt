cmake_minimum_required(VERSION 3.10)

# Define the project name and specify the required C++ standard
project(UfWebCameraV4L)
find_package(OpenCV REQUIRED)

# Specify the version (optional)
set(PROJECT_VERSION 1.0)

# Set the C++ standard for the project (you can change this if needed)
set(CMAKE_CXX_STANDARD 17)

set(UF_NAME ${CMAKE_PROJECT_NAME})
set(EXE_NAME "${CMAKE_PROJECT_NAME}_TEST")

# Specify the source directory
set(AVEXECUTOR_DIR "/etc/avexecutor/")
set(AVEXECUTOR_DIR_LIB_DIR "${AVEXECUTOR_DIR}lib/x86_64-linux-gnu/")
set(AVEXECUTOR_DIR_LIB_INCLUDE "${AVEXECUTOR_DIR}include/")

set(HEAD_DIR "./headers")
set(SRC_DIR "./sources")



# Collect all .cpp files in the src directory
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Specify the include directories for avexecutor
include_directories(${AVEXECUTOR_DIR_LIB_INCLUDE})

# Include directories for headers
include_directories(${HEAD_DIR})

# Specify the library directories for avexecutor and other dependencies
link_directories(${AVEXECUTOR_DIR_LIB_DIR})

# Add the shared library target
add_library(${UF_NAME} SHARED ${SRC_FILES})
add_executable(${EXE_NAME} sources/test.cpp ${SRC_FILES})

# Link the necessary libraries
target_link_libraries(${UF_NAME}
    AVL_Lite  # Assuming this is available in the specified library directory
    UserFilters  # Assuming this is available in the specified library directory
    ${OpenCV_LIBS}
)
target_link_libraries(${EXE_NAME}
    AVL_Lite  # Assuming this is available in the specified library directory
    UserFilters  # Assuming this is available in the specified library directory
    ${OpenCV_LIBS}

)

# Set the output name for the shared library
set_target_properties(${UF_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if(WIN32)
    # Windows-specific source files or definitions
    target_compile_definitions(${UF_NAME} PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(${EXE_NAME} PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
    # Unix-like (Linux, macOS) specific source files or definitions
    target_compile_definitions(${UF_NAME} PRIVATE PLATFORM_UNIX)
    target_compile_definitions(${EXE_NAME} PRIVATE PLATFORM_UNIX)
endif()

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
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/*.so"
    COMMENT "Removing CMake-generated files and the executable"
)