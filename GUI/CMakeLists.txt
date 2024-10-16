cmake_minimum_required(VERSION 3.0)
project(AstroPhotoStackerGUI)

find_package(wxWidgets REQUIRED COMPONENTS net core base adv)
find_package(OpenCV)
find_package(exiv2 REQUIRED CONFIG)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})


include(${wxWidgets_USE_FILE})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bin/)

message(${wxWidgets_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")
set(LINKER_FLAGS "-lexiv2 -lraw")

file(GLOB SOURCES "src/*.cxx")

add_library(AstroPhotoStackerGUILib SHARED ${SOURCES})

add_executable(AstroPhotoStackerGUI "utils/main.cxx")
add_executable(tests "utils/tests.cxx")
add_executable(image_viewer "utils/image_viewer.cxx")

target_link_libraries(AstroPhotoStackerGUI AstroPhotoStackerGUILib ${wxWidgets_LIBRARIES} ${OpenCV_LIBS}  ${CMAKE_CURRENT_SOURCE_DIR}/../bin/libAstroPhotoStackerLib.so ${LINKER_FLAGS})
target_link_libraries(tests AstroPhotoStackerGUILib ${wxWidgets_LIBRARIES} ${OpenCV_LIBS}  ${CMAKE_CURRENT_SOURCE_DIR}/../bin/libAstroPhotoStackerLib.so ${LINKER_FLAGS})
target_link_libraries(image_viewer AstroPhotoStackerGUILib ${wxWidgets_LIBRARIES} ${OpenCV_LIBS}  ${CMAKE_CURRENT_SOURCE_DIR}/../bin/libAstroPhotoStackerLib.so ${LINKER_FLAGS})