cmake_minimum_required(VERSION 3.15)

project(mcpelauncher-imghost LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(mcpelauncher-imghost SHARED
    src/main.cpp
    src/config.cpp
    src/ghost.cpp
    src/input.cpp
    src/menu.cpp
)

target_include_directories(mcpelauncher-imghost PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# log = Android liblog, dl = dlopen/dlsym
target_link_libraries(mcpelauncher-imghost PRIVATE log dl)

set_target_properties(mcpelauncher-imghost PROPERTIES
    OUTPUT_NAME "mcpelauncher-imghost"
)

install(TARGETS mcpelauncher-imghost LIBRARY DESTINATION mods)
