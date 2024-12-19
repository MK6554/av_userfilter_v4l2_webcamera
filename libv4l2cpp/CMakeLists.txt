cmake_minimum_required(VERSION 3.0)

# set project name from current directory
get_filename_component(BASENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${BASENAME})

set (CMAKE_CXX_STANDARD 11)

#aux_source_directory(src SRC_FILES)
# Collect all .cpp files in the src directory
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/inc")

add_executable (${PROJECT_NAME}test main.cpp)
target_link_libraries (${PROJECT_NAME}test ${PROJECT_NAME}) 

