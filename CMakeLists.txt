cmake_minimum_required (VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

project ("Application" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_ATANDARD_REQUIRED ON)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(glad REQUIRED)
find_package(Stb REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(apps)
add_subdirectory(base)
add_subdirectory(opengl)
add_subdirectory(leetcode)
add_subdirectory(ffmpeg)
