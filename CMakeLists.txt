cmake_minimum_required(VERSION 3.30)
project(j3)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_UNITY_BUILD TRUE)

# for DirectXTK
set(DIRECTX_ARCH x64)

# for RmlUi
set(RMLUI_BACKEND Win32_DX11)
set(BUILD_SHARED_LIBS OFF)

### dependencies

CPMAddPackage("gh:gabime/spdlog@1.15.3")
CPMAddPackage("gh:skypjack/entt@3.15.0")
CPMAddPackage("gh:microsoft/DirectXTK#mar2025")

add_subdirectory(tools/freetype)
add_library(Freetype::Freetype ALIAS freetype)

FetchContent_Declare(
        RmlUi
        GIT_REPOSITORY https://github.com/phasephasephase/RmlUi.git
        GIT_TAG 338ce6c2d11d26481fba5c6f78239910ccd19018
)

FetchContent_Declare(
        harfbuzz
        GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
        GIT_TAG 11.2.1
)

FetchContent_MakeAvailable(spdlog entt DirectXTK RmlUi harfbuzz)

### source files

# create obj directory for intermediate files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/obj)

# define common folders
set(OBJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/obj)
set(TOOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)

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

# all resources except shaders get embedded into the executable
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS "resources/*")
list(FILTER RESOURCES EXCLUDE REGEX "resources/shaders/*")

add_resources(RESOURCE_OBJECTS
        ${INTERMEDIARIES}
        ${RESOURCES}
)

### exe

add_executable(j3 WIN32 ${SOURCES} ${SHADER_OUTPUTS} ${RESOURCE_OBJECTS})

### includes and libraries

include_directories(src)
target_precompile_headers(j3 PRIVATE src/common.hpp)

# disable unity build for DirectXTK and RmlUi
set_property(TARGET DirectXTK rmlui_core rmlui_debugger freetype PROPERTY UNITY_BUILD FALSE)

# RmlUi backend stuff
set(RMLUI_BACKEND_PATH ${RmlUi_SOURCE_DIR}/Backends)
add_library(rmlui_backend_WIN32_DX11 INTERFACE)
target_sources(rmlui_backend_WIN32_DX11 INTERFACE
        ${RMLUI_BACKEND_PATH}/RmlUi_Platform_Win32.cpp
        ${RMLUI_BACKEND_PATH}/RmlUi_Renderer_DX11.cpp
)
target_include_directories(rmlui_backend_WIN32_DX11 INTERFACE ${RMLUI_BACKEND_PATH})

# include private RmlUi Core headers to make harfbuzz work
target_include_directories(j3 PRIVATE ${RmlUi_SOURCE_DIR}/Source/Core)

target_link_libraries(j3 PRIVATE spdlog::spdlog EnTT::EnTT DirectXTK RmlUi::RmlUi rmlui_backend_WIN32_DX11 harfbuzz d3d11 dxgi d3dcompiler)

target_compile_definitions(j3 PRIVATE UNICODE)
target_compile_definitions(j3 PRIVATE WINDOW_CLASS_NAME=L"j3window")
