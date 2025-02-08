cmake_minimum_required(VERSION 3.16)
project(My3DRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)

# Define the executable target
add_executable(My3DRenderer
    includes/glad/src/glad.c
    includes/imgui/backends/imgui_impl_glfw.cpp
    includes/imgui/backends/imgui_impl_opengl3.cpp
    includes/imgui/imgui.cpp
    includes/imgui/imgui_demo.cpp
    includes/imgui/imgui_draw.cpp
    includes/imgui/imgui_tables.cpp
    includes/imgui/imgui_widgets.cpp
    includes/stb_image/stb_image.cpp
    main.cpp
)

# Include directories
target_include_directories(My3DRenderer PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/includes/glfw/include
    ${CMAKE_SOURCE_DIR}/includes/glad/include
    ${CMAKE_SOURCE_DIR}/includes/assimp/include
    ${CMAKE_SOURCE_DIR}/includes/imgui
    ${CMAKE_SOURCE_DIR}/includes/glm
    ${CMAKE_SOURCE_DIR}/includes/stb_image
)

# Link libraries
add_subdirectory(includes/assimp)

set(LIBS glfw3)
target_link_libraries(My3DRenderer PRIVATE ${LIBS})

set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/includes/glfw/include)
target_link_directories(My3DRenderer PRIVATE ${GLFW_LIB_PATH})

target_include_directories(My3DRenderer PRIVATE ${CMAKE_SOURCE_DIR}/includes/assimp)
target_link_libraries(My3DRenderer PRIVATE assimp)

# Platform-specific settings
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_ARCH x64)
else()
    set(TARGET_ARCH Win32)
endif()

# Compiler settings
if (MSVC)
    target_compile_definitions(My3DRenderer PRIVATE $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:Release>:NDEBUG>)
    target_compile_options(My3DRenderer PRIVATE /W3 /permissive- /utf-8)
endif()

add_custom_command(TARGET My3DRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:My3DRenderer>/Assets
)