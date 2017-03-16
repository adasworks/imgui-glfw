cmake_minimum_required(VERSION 3.0)

project(imgui_demo)

if(NOT TARGET ::imgui)
    find_package(imgui REQUIRED)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    imgui_demo.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE ::imgui)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
