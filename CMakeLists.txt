cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DarkCirclesBuff VERSION 1.0.0)

# ── Point this at your Geode CLI / SDK location if needed ──
# set(GEODE_SDK "C:/path/to/Geode")

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable not set. "
        "Run `geode sdk install` or set the variable manually.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# ── Mod target ──
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} geode-sdk)

setup_geode_mod(${PROJECT_NAME})
