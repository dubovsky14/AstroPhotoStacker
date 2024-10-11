cmake_minimum_required(VERSION 3.0)
project(AstroPhotoStackerGUI)

find_package(wxWidgets REQUIRED COMPONENTS net core base adv)
find_package(OpenCV)
find_package(exiv2 REQUIRED CONFIG)

include(${wxWidgets_USE_FILE})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bin/)

message(${wxWidgets_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")
set(LINKER_FLAGS "-lexiv2 -lraw")

file(GLOB SOURCES "src/*.cxx")

add_executable(AstroPhotoStackerGUI ${SOURCES} "utils/main.cxx")
add_executable(tests ${SOURCES} "utils/tests.cxx")

target_link_libraries(AstroPhotoStackerGUI  ${wxWidgets_LIBRARIES} ${OpenCV_LIBS}  ${CMAKE_CURRENT_SOURCE_DIR}/../bin/libAstroPhotoStackerLib.so ${LINKER_FLAGS})
target_link_libraries(tests  ${wxWidgets_LIBRARIES} ${OpenCV_LIBS}  ${CMAKE_CURRENT_SOURCE_DIR}/../bin/libAstroPhotoStackerLib.so ${LINKER_FLAGS})