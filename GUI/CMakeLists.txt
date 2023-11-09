cmake_minimum_required(VERSION 3.0)
project(AstroPhotoStackerGUI)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

message(${wxWidgets_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")

file(GLOB SOURCES "src/*.cxx")

add_executable(AstroPhotoStackerGUI ${SOURCES} "utils/main.cxx")

target_link_libraries(AstroPhotoStackerGUI ${wxWidgets_LIBRARIES})