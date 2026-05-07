cmake_minimum_required(VERSION 3.30)
project(j3)

set(CPM_SOURCE_CACHE ${PROJECT_SOURCE_DIR}/cpm-cache)
include(cmake/get_cpm.cmake)

set(CMAKE_CXX_STANDARD 23)

# for DirectXTK
set(DIRECTX_ARCH x64)

# for RmlUi
set(RMLUI_BACKEND Win32_DX11)
set(RMLUI_SVG_PLUGIN ON)
set(BUILD_SHARED_LIBS OFF)

# for Assimp
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

### dependencies

CPMAddPackage("gh:gabime/spdlog@1.17.0")
CPMAddPackage("gh:skypjack/entt@3.16.0")
CPMAddPackage("gh:microsoft/DirectXTK#oct2025")
CPMAddPackage("https://gitlab.freedesktop.org/freetype/freetype.git#VER-2-14-3")
CPMAddPackage("gh:harfbuzz/harfbuzz#13.2.1")
CPMAddPackage("gh:assimp/assimp@6.0.2")
CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:stephenberry/glaze@7.2.2")
CPMAddPackage("gh:cameron314/concurrentqueue@1.0.5")
CPMAddPackage("gh:sammycage/lunasvg@3.5.0")

# define alias for freetype so RmlUi can find it
add_library(Freetype::Freetype ALIAS freetype)
CPMAddPackage("gh:JiayiSoftware/RmlUi#dx11-backend")

### source files

# create obj directory for intermediate files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/obj)

# define common folders
set(OBJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/obj)
set(TOOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

file(GLOB VERTEX_SHADERS CONFIGURE_DEPENDS "resources/shaders/*.vs.hlsl")
file(GLOB PIXEL_SHADERS CONFIGURE_DEPENDS "resources/shaders/*.ps.hlsl")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")

set_source_files_properties(${VERTEX_SHADERS} PROPERTIES ShaderType "vs")
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES ShaderType "ps")
set_source_files_properties(${VERTEX_SHADERS} ${PIXEL_SHADERS} PROPERTIES ShaderModel "5_0")

### custom functions

function(ADD_SHADERS out_var)
    set(result)
    foreach(FILE ${ARGN})
        get_filename_component(FILE_WE ${FILE} NAME_WLE)
        get_source_file_property(SHADER_TYPE ${FILE} ShaderType)
        get_source_file_property(SHADER_MODEL ${FILE} ShaderModel)
        add_custom_command(
                OUTPUT ${OBJ_DIR}/${FILE_WE}.cso
                COMMAND fxc.exe /nologo /Emain /T${SHADER_TYPE}_${SHADER_MODEL} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Fo ${OBJ_DIR}/${FILE_WE}.cso ${FILE}
                DEPENDS ${FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building shader ${FILE_WE}.cso"
                VERBATIM
        )
        list(APPEND result ${OBJ_DIR}/${FILE_WE}.cso)
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

function(ADD_RESOURCES out_var)
    set(result)
    foreach(in_f ${ARGN})
        file(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${in_f})
        get_filename_component(in_f_name ${in_f} NAME)
        set(out_f "${PROJECT_BINARY_DIR}/${in_f_name}.o")
        add_custom_command(
                OUTPUT ${out_f}
                COMMAND ${TOOL_DIR}/ld.exe -r -b binary -o ${out_f} ${src_f}
                DEPENDS ${in_f}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Adding resource ${in_f}"
                VERBATIM
        )
        list(APPEND result ${out_f})
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

### shaders

add_shaders(SHADER_OUTPUTS ${VERTEX_SHADERS} ${PIXEL_SHADERS})

### resources

file(GLOB_RECURSE INTERMEDIARIES CONFIGURE_DEPENDS "obj/*")
add_resources(RESOURCE_OBJECTS
        ${INTERMEDIARIES}
)

# all resources except shaders get copied to the output directory
file(COPY ${RESOURCE_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

### exe

add_executable(j3 WIN32 ${SOURCES} ${SHADER_OUTPUTS} ${RESOURCE_OBJECTS} app.manifest)
add_compile_options("/MP $<$<CONFIG:Release>:/clang:-flto=thin /clang:-Oz>")

### includes and libraries

include_directories(src)
target_precompile_headers(j3 PRIVATE src/common.hpp)

# RmlUi DX11 backend
set(RMLUI_BACKEND_PATH ${RmlUi_SOURCE_DIR}/Backends)
add_library(rmlui_backend_WIN32_DX11 INTERFACE)

target_sources(rmlui_backend_WIN32_DX11 INTERFACE 
    ${RMLUI_BACKEND_PATH}/RmlUi_Platform_Win32.cpp
    ${RMLUI_BACKEND_PATH}/RmlUi_Renderer_DX11.cpp
)

target_include_directories(rmlui_backend_WIN32_DX11 INTERFACE ${RMLUI_BACKEND_PATH})

# RmlUi harfbuzz font engine
set(RMLUI_HARFBUZZ_PATH ${RmlUi_SOURCE_DIR}/Samples/basic/harfbuzz/src)
add_library(rmlui_font_engine_harfbuzz INTERFACE)

file(GLOB RMLUI_HARFBUZZ_SOURCES ${RMLUI_HARFBUZZ_PATH}/*.cpp ${RMLUI_HARFBUZZ_PATH}/*.h)
list(FILTER RMLUI_HARFBUZZ_SOURCES EXCLUDE REGEX "main.cpp")
target_sources(rmlui_font_engine_harfbuzz INTERFACE ${RMLUI_HARFBUZZ_SOURCES})

target_include_directories(rmlui_font_engine_harfbuzz INTERFACE ${RmlUi_SOURCE_DIR}/Source/Core)
include_directories(${RMLUI_HARFBUZZ_PATH})

target_link_libraries(j3 PRIVATE 
        spdlog::spdlog
        EnTT::EnTT
        DirectXTK
        RmlUi::RmlUi
        rmlui_backend_WIN32_DX11
        rmlui_font_engine_harfbuzz
        harfbuzz
        assimp
        libhat
        glaze::glaze
        concurrentqueue
        d3d11
        dxgi
        d3dcompiler
        dwmapi
)

target_compile_definitions(j3 PRIVATE UNICODE)
target_compile_definitions(j3 PRIVATE NOMINMAX)
target_compile_definitions(j3 PRIVATE WINDOW_CLASS_NAME=L"j3window")
