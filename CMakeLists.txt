cmake_minimum_required(VERSION 3.0)

project(imgui)

set(imgui_dir ${CMAKE_CURRENT_LIST_DIR})

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

set(imgui_public_hdrs
    ${imgui_dir}/imgui.h
    ${imgui_dir}/imgui_internal.h
    ${imgui_dir}/imconfig.h
    ${imgui_dir}/imstb_rectpack.h
    ${imgui_dir}/imstb_textedit.h
    ${imgui_dir}/imstb_truetype.h
    ${imgui_dir}/implot/implot.h
    ${imgui_dir}/implot/implot_internal.h
    ${imgui_dir}/imgui_glfw.h
)

add_library(imgui
    ${imgui_public_hdrs}
    ${imgui_dir}/implot/implot.cpp
    ${imgui_dir}/implot/implot_items.cpp
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui_draw.cpp
    ${imgui_dir}/imgui_widgets.cpp
    ${imgui_dir}/imgui_glfw.cpp
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
if (WIN32)
    find_path(GLES2_INCLUDE_DIRS GLES2/gl2.h)
    target_include_directories(imgui PRIVATE ${GLES2_INCLUDE_DIRS})
endif()
set_target_properties(imgui PROPERTIES DEBUG_POSTFIX d)
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
