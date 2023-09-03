# Project Info
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)
project(mrender)
enable_language(C)
enable_language(CXX)    

# CMake Settings
set(CMAKE_SUPPRESS_REGENERATION true)
set(DCMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/**.hpp
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "mrender" "" source_path_final "${source_path_msvc}")
    source_group("${source_path_final}" FILES "${source}")
endforeach()

# Finalize Library
add_library(
    ${PROJECT_NAME}
    ${FILE_SOURCES}
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Dependencies ================================================

# BGFX
add_subdirectory(3rdparty/bgfx)
target_link_libraries(${PROJECT_NAME} PRIVATE bgfx bx)

set(MRENDER_BGFX_TARGETS
    bgfx
    bx
    bimg
    example-common
    examples
    shaderc
    tools
    bimg_decode
    bimg_encode
    fcpp
    glslang
    glsl-optimizer
    spirv-cross
    spirv-opt
)
foreach(target ${MRENDER_BGFX_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "mrender /bgfx ")
endforeach()

# =============================================================

