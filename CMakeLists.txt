cmake_minimum_required(VERSION 3.30)
project(j3)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_UNITY_BUILD TRUE)
set(DIRECTX_ARCH x64) # for DirectXTK

### dependencies

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.1
)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.14.0
)

FetchContent_Declare(
        DirectXTK
        GIT_REPOSITORY https://github.com/microsoft/DirectXTK.git
        GIT_TAG oct2024
)

FetchContent_MakeAvailable(spdlog entt DirectXTK)

### source files

# create obj directory for intermediate files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/obj)

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
        add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/obj/${FILE_WE}.cso
                COMMAND fxc.exe /nologo /Emain /T${SHADER_TYPE}_${SHADER_MODEL} $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Fo ${CMAKE_CURRENT_SOURCE_DIR}/obj/${FILE_WE}.cso ${FILE}
                DEPENDS ${FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building shader ${FILE_WE}.cso"
                VERBATIM
        )
        list(APPEND result ${CMAKE_CURRENT_SOURCE_DIR}/obj/${FILE_WE}.cso)
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

function(ADD_RESOURCES out_var)
    set(result)
    foreach(in_f ${ARGN})
        file(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${in_f})
        get_filename_component(in_f_name ${in_f} NAME)
        set(out_f "${PROJECT_BINARY_DIR}/${in_f_name}.o")
        add_custom_command(OUTPUT ${out_f}
                COMMAND ld -r -b binary -o ${out_f} ${src_f}
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

# disable unity build for DirectXTK
set_property(TARGET DirectXTK PROPERTY UNITY_BUILD FALSE)

target_link_libraries(j3 PRIVATE spdlog::spdlog EnTT::EnTT DirectXTK d3d11 dxgi d3dcompiler)

target_compile_definitions(j3 PRIVATE UNICODE)
target_compile_definitions(j3 PRIVATE WINDOW_CLASS_NAME=L"j3window")
