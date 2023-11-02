cmake_minimum_required(VERSION 3.0)
project(AstroPhotoStacker)
find_package(OpenCV)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")


file(GLOB SOURCES "src/*.cxx")


add_library(AstroPhotoStackerLib SHARED ${SOURCES})

file(GLOB CI_TESTS_SOURCES "CI_tests/src/*.cxx")

# add the executable
add_executable(AstroPhotoStacker utils/main.cxx)
add_executable(produce_alignment_file utils/produce_alignment_file.cxx)
add_executable(convert_raw_to_jpg utils/convert_raw_to_jpg.cxx)
add_executable(identify_hot_pixels utils/identify_hot_pixels.cxx)
add_executable(stack_calibration_frames utils/stack_calibration_frames.cxx)
add_executable(tests ${CI_TESTS_SOURCES} "utils/tests.cxx")

add_dependencies(AstroPhotoStacker AstroPhotoStackerLib)
add_dependencies(produce_alignment_file AstroPhotoStackerLib)
add_dependencies(convert_raw_to_jpg AstroPhotoStackerLib)
add_dependencies(identify_hot_pixels AstroPhotoStackerLib)
add_dependencies(tests AstroPhotoStackerLib)
add_dependencies(stack_calibration_frames AstroPhotoStackerLib)

# link the libraw library
target_link_libraries(AstroPhotoStacker AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(produce_alignment_file AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(convert_raw_to_jpg AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(identify_hot_pixels AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(tests AstroPhotoStackerLib -lraw ${OpenCV_LIBS})
target_link_libraries(stack_calibration_frames AstroPhotoStackerLib -lraw ${OpenCV_LIBS})