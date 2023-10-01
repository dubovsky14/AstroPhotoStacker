cmake_minimum_required(VERSION 3.0)
find_package(OpenCV)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

project(AstroPhotoStacker)

file(GLOB SOURCES "src/*.cxx")


add_library(AstroPhotoStackerLib SHARED ${SOURCES})


# add the executable
add_executable(AstroPhotoStacker utils/main.cxx)

add_dependencies(AstroPhotoStacker AstroPhotoStackerLib)

# link the libraw library
target_link_libraries(AstroPhotoStacker AstroPhotoStackerLib -lraw ${OpenCV_LIBS})