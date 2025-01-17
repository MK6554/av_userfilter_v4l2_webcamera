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
#if(WIN32)
#    set(AVL_LITE_DIR "$ENV{AVS_PROFESSIONAL_SDK5_5}\\")
#    set(AVL_LITE_LIB_DIR "${AVL_LITE_DIR}lib\\x64\\")
#    set(AVL_LITE_INCLUDE_DIR "${AVL_LITE_DIR}include\\")
#    set(USER_PROPS_PATH "${AVL_LITE_DIR}props\\UserFilters_v143.props")
#    set(SRC_PLATFORM)
#    add_compile_definitions(PLATFORM_WINDOWS)
#elseif(UNIX)
    #find_package(OpenCV REQUIRED)
    set(AVL_LITE_DIR "/etc/avexecutor/")
    set(AVL_LITE_LIB_DIR "${AVL_LITE_DIR}lib/x86_64-linux-gnu/")
    set(AVL_LITE_INCLUDE_DIR "${AVL_LITE_DIR}include/")
    set(SRC_PLATFORM
        ./sources/paraqueue.cpp
        ./sources/webcamera.cpp
    )
#    add_compile_definitions(PLATFORM_UNIX)
#endif()

set(SRC_FILES ${SRC_CORE} ${SRC_PLATFORM})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Collect all .cpp files in the src directory
# file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")
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

#if(UNIX)
#    add_executable(${EXE_NAME} sources/test.cpp ${SRC_FILES})
#endif()

# set_property(TARGET ${UF_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})

# Link the necessary libraries
target_link_libraries(${UF_NAME} PUBLIC
    AVL_Lite # Assuming this is available in the specified library directory
    UserFilters # Assuming this is available in the specified library directory
    libv4l2cpp#  <------- linked only for UNIX
)

# Set the output name for the shared library
set_target_properties(${UF_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

#if(WIN32)
#    # Windows-specific source files or definitions
#    set_target_properties(${UF_NAME} PROPERTIES VS_USER_PROPS "${USER_PROPS_PATH}")
#elseif(UNIX)
    # Unix-like (Linux, macOS) specific source files or definitions
    #target_link_libraries(${UF_NAME}
    #    AVL_Lite # Assuming this is available in the specified library directory
    #    UserFilters # Assuming this is available in the specified library directory
    #    libv4l2cpp
    #)
    #set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    #target_link_libraries(${UF_NAME} PUBLIC ${OpenCV_LIBS})
    #target_link_libraries(${EXE_NAME} ${OpenCV_LIBS})
#endif()

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
