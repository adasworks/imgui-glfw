cmake_minimum_required(VERSION 3.0)

project(imgui_demo)

if(NOT TARGET ::imgui)
    find_package(imgui REQUIRED)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    ../../imgui_demo.cpp
    ../../implot/implot_demo.cpp
)

find_library(GLES2_LIBRARY NAMES GLESv2 libGLESv2.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ::imgui ${GLES2_LIBRARY})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
