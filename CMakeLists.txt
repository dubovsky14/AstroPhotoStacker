cmake_minimum_required(VERSION 3.0)
project(AstroPhotoStacker)
find_package(OpenCV)
find_package(exiv2)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-Wall -O3")


file(GLOB SOURCES "src/*.cxx")

#linker flags
set(LINKER_FLAGS "-lexiv2 -lraw")

add_library(AstroPhotoStackerLib SHARED ${SOURCES})
target_link_libraries(AstroPhotoStackerLib PUBLIC ${LINKER_FLAGS} ${OpenCV_LIBS})


# Find all .cxx files in the utils folder
file(GLOB UTILS_SOURCES "utils/*.cxx")

# Iterate over each .cxx file
foreach(SOURCE_FILE ${UTILS_SOURCES})
    # Get the filename without the extension
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)

    # Add executable
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})

    # Add dependencies
    add_dependencies(${EXECUTABLE_NAME} AstroPhotoStackerLib)

    # Link libraries
    target_link_libraries(${EXECUTABLE_NAME} AstroPhotoStackerLib ${LINKER_FLAGS} ${OpenCV_LIBS})
endforeach()

# CI tests
file(GLOB CI_TESTS_SOURCES "CI_tests/src/*.cxx")
add_executable(tests ${CI_TESTS_SOURCES} "CI_tests/utils/tests.cxx")
add_dependencies(tests AstroPhotoStackerLib)
target_link_libraries(tests AstroPhotoStackerLib ${LINKER_FLAGS} ${OpenCV_LIBS})
