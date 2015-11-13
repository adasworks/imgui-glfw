cmake_minimum_required(VERSION 3.0)

project(imgui)

set(imgui_dir ${CMAKE_CURRENT_LIST_DIR})

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew QUIET CONFIG)

set(imgui_public_hdrs
    ${imgui_dir}/imgui.h
    ${imgui_dir}/imconfig.h
    ${imgui_dir}/imgui_glfw.h
    ${imgui_dir}/stb_rect_pack.h
    ${imgui_dir}/stb_textedit.h
    ${imgui_dir}/stb_truetype.h
)

add_library(imgui
    ${imgui_public_hdrs}
    ${imgui_dir}/imgui_internal.h
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui_draw.cpp

    imgui_glfw.cpp
)
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${imgui_dir}>
    $<INSTALL_INTERFACE:include/imgui>
)
target_link_libraries(imgui
    glm
    glfw
)
if (glew_FOUND)
    target_link_libraries(imgui GLEW::glew_s)
    target_compile_definitions(imgui PRIVATE -DHAVE_GLEW)
endif()
install(TARGETS imgui EXPORT imgui-target DESTINATION lib)
install(FILES ${imgui_public_hdrs} DESTINATION include/imgui)
install(EXPORT imgui-target DESTINATION lib/cmake/imgui FILE imgui-config.cmake)
