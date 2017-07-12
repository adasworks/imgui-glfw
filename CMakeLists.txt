cmake_minimum_required(VERSION 3.0)

project(imgui)

set(imgui_dir ${CMAKE_CURRENT_LIST_DIR})

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW QUIET CONFIG)

set(imgui_public_hdrs
    ${imgui_dir}/imgui.h
    ${imgui_dir}/imconfig.h
    ${imgui_dir}/imgui_glfw.h
    ${imgui_dir}/stb_rect_pack.h
    ${imgui_dir}/stb_textedit.h
    ${imgui_dir}/stb_truetype.h
    ${imgui_dir}/imgui_tabs/imgui_tabs.h
)

add_library(imgui
    ${imgui_public_hdrs}
    ${imgui_dir}/imgui_internal.h
    ${imgui_dir}/imgui_tabs/imgui_tabs.cpp
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui_draw.cpp
    imgui_glfw.cpp
)
add_library(::imgui ALIAS imgui)
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${imgui_dir}>
    $<INSTALL_INTERFACE:include/imgui>
)
target_link_libraries(imgui
    PUBLIC glfw
    PRIVATE glm
)
set_target_properties(imgui PROPERTIES DEBUG_POSTFIX d)
if (GLEW_FOUND)
    message(STATUS "GLEW found")
    target_link_libraries(imgui PRIVATE GLEW::glew_s)
    target_compile_definitions(imgui PRIVATE -DHAVE_GLEW)
else()
    message(STATUS "GLEW not found")
endif()
configure_file(imgui-config.cmake.in imgui-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/imgui-config.cmake DESTINATION lib/cmake/imgui)
install(TARGETS imgui EXPORT imgui-targets DESTINATION lib)
install(FILES ${imgui_public_hdrs} DESTINATION include/imgui)
install(EXPORT imgui-targets NAMESPACE :: DESTINATION lib/cmake/imgui)

option(BUILD_SAMPLES "Build example projects with cmake support" ON)
if (BUILD_SAMPLES)
    add_subdirectory(examples/cmake)
    install(DIRECTORY examples/cmake/ DESTINATION examples/imgui_demo)
endif()
