cmake_minimum_required(VERSION 3.0)
find_package(OpenCV REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")

project(AstroPhotoStacker)

file(GLOB SOURCES "src/*.cxx")


add_library(AstroPhotoStackerLib SHARED ${SOURCES})


# add the executable
add_executable(AstroPhotoStacker utils/main.cxx)
add_executable(produce_alignment_file utils/produce_alignment_file.cxx)

add_dependencies(AstroPhotoStacker AstroPhotoStackerLib)
add_dependencies(produce_alignment_file AstroPhotoStackerLib)

# link the libraw library
target_link_libraries(AstroPhotoStacker AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(produce_alignment_file AstroPhotoStackerLib -lraw ${OpenCV_LIBS})