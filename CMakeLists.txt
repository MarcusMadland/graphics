# To prevent this warning: https://cmake.org/cmake/help/git-stage/policy/CMP0072.html
if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

if(NOT APPLE)
	set(GRAPHICS_AMALGAMATED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/src/amalgamated.cpp)
else()
	set(GRAPHICS_AMALGAMATED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/src/amalgamated.mm)
endif()

# Sources
set(FCPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp)
file(
	GLOB_RECURSE
	GRAPHICS_SOURCES

	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/**.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/**.mm
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/**.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/include/**.h
##
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp1.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp2.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp3.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp4.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp5.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp6.c
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/cpp6.c
)

# Solution Filters
foreach(_source IN ITEMS ${GRAPHICS_SOURCES})
	if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
endforeach()

# Do not build using amalgamated sources
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/amalgamated.cpp PROPERTIES HEADER_FILE_ONLY ON)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/amalgamated.mm PROPERTIES HEADER_FILE_ONLY ON)

# Create the graphics target
if(GRAPHICS_LIBRARY_TYPE STREQUAL STATIC)
	add_library(graphics STATIC ${GRAPHICS_SOURCES})
else()
	add_library(graphics SHARED ${GRAPHICS_SOURCES})
	target_compile_definitions(graphics PUBLIC GRAPHICS_SHARED_LIB_BUILD=1)
endif()

if(EMSCRIPTEN)
	target_link_options(graphics PUBLIC "-sMAX_WEBGL_VERSION=2")
endif()

if(NOT ${GRAPHICS_OPENGL_VERSION} STREQUAL "")
	target_compile_definitions(graphics PRIVATE GRAPHICS_CONFIG_RENDERER_OPENGL_MIN_VERSION=${GRAPHICS_OPENGL_VERSION})
endif()

if(NOT ${GRAPHICS_OPENGLES_VERSION} STREQUAL "")
	target_compile_definitions(graphics PRIVATE GRAPHICS_CONFIG_RENDERER_OPENGLES_MIN_VERSION=${GRAPHICS_OPENGLES_VERSION})
endif()

if(NOT ${GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS} STREQUAL "")
	target_compile_definitions(
		graphics
		PUBLIC
			"GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS=$<IF:$<BOOL:${GRAPHICS_CONFIG_MULTITHREADED}>,${GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS},1>"
	)
endif()

set(GRAPHICS_CONFIG_OPTIONS "")
list(
	APPEND
	GRAPHICS_CONFIG_OPTIONS
	"GRAPHICS_CONFIG_MAX_DRAW_CALLS"
	"GRAPHICS_CONFIG_MAX_VIEWS"
	"GRAPHICS_CONFIG_MAX_FRAME_BUFFERS"
	"GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS"
	"GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS"
	"GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS"
	"GRAPHICS_CONFIG_MAX_INDEX_BUFFERS"
	"GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS"
	"GRAPHICS_CONFIG_MAX_TEXTURES"
	"GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS"
	"GRAPHICS_CONFIG_MAX_SHADERS"
	"GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM"
)
foreach(GRAPHICS_CONFIG_OPTION IN LISTS GRAPHICS_CONFIG_OPTIONS)
	if(NOT ${${GRAPHICS_CONFIG_OPTION}} STREQUAL "")
		target_compile_definitions(graphics PUBLIC "${GRAPHICS_CONFIG_OPTION}=${${GRAPHICS_CONFIG_OPTION}}")
	endif()
endforeach()

# Special Visual Studio Flags
if(MSVC)
	target_compile_definitions(graphics PUBLIC "_CRT_SECURE_NO_WARNINGS")
endif()

# 
target_compile_definitions(
	graphics
	PUBLIC
		"BASE_CONFIG_DEBUG=$<OR:$<CONFIG:Debug>,$<BOOL:${BASE_CONFIG_DEBUG}>>"
		"GRAPHICS_CONFIG_DEBUG_ANNOTATION=$<AND:$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},WindowsStore>>,$<OR:$<CONFIG:Debug>,$<BOOL:${GRAPHICS_CONFIG_DEBUG_ANNOTATION}>>>"
		"GRAPHICS_CONFIG_MULTITHREADED=$<BOOL:${GRAPHICS_CONFIG_MULTITHREADED}>"
)

# directx-headers
set(DIRECTX_HEADERS)
if(UNIX
   AND NOT APPLE
   AND NOT EMSCRIPTEN
   AND NOT ANDROID
) # Only Linux
	set(DIRECTX_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/directx-headers/include/directx ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/directx-headers/include
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/directx-headers/include/wsl/stubs
	)
elseif(WIN32) # Only Windows
	set(DIRECTX_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/directx-headers/include/directx
						${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/directx-headers/include
	)
endif()

# Includes
target_include_directories(
	graphics PRIVATE ${DIRECTX_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/khronos 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/base/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Dependencies
target_link_libraries(graphics PUBLIC base)

# Frameworks required on iOS, tvOS and macOS
if(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
	target_link_libraries(
		graphics
		PUBLIC
			"-framework OpenGLES -framework Metal -framework UIKit -framework CoreGraphics -framework QuartzCore -framework IOKit -framework CoreFoundation"
	)
elseif(APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	find_library(METAL_LIBRARY Metal)
	find_library(QUARTZCORE_LIBRARY QuartzCore)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	mark_as_advanced(COCOA_LIBRARY)
	mark_as_advanced(METAL_LIBRARY)
	mark_as_advanced(QUARTZCORE_LIBRARY)
	mark_as_advanced(IOKIT_LIBRARY)
	mark_as_advanced(COREFOUNDATION_LIBRARY)
	target_link_libraries(
		graphics PUBLIC ${COCOA_LIBRARY} ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY} ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY}
	)
endif()

if(UNIX
   AND NOT APPLE
   AND NOT EMSCRIPTEN
   AND NOT ANDROID
)
	find_package(X11 REQUIRED)
	find_package(OpenGL REQUIRED)
	target_link_libraries(graphics PUBLIC ${X11_LIBRARIES} ${OPENGL_LIBRARIES})
endif()

# Exclude mm files if not on OS X
if(NOT APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/glcontext_eagl.mm PROPERTIES HEADER_FILE_ONLY ON)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/glcontext_nsgl.mm PROPERTIES HEADER_FILE_ONLY ON)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/renderer_mtl.mm PROPERTIES HEADER_FILE_ONLY ON)
endif()

# Exclude glx context on non-unix
if(NOT UNIX OR APPLE)
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/src/glcontext_glx.cpp PROPERTIES HEADER_FILE_ONLY ON)
endif()

# in Xcode we need to specify this file as objective-c++ (instead of renaming to .mm)
if(XCODE)
	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/source/src/renderer_vk.cpp PROPERTIES LANGUAGE OBJCXX XCODE_EXPLICIT_FILE_TYPE sourcecode.cpp.objcpp
	)
endif()

## FCPP
target_include_directories(graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fcpp/)

# Dependencies
add_subdirectory(base)

# Put in a "graphics" folder in Visual Studio
set_target_properties(graphics PROPERTIES FOLDER "graphics ")
set_target_properties(base PROPERTIES FOLDER "graphics ")