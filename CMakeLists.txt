cmake_minimum_required(VERSION 2.8)

PROJECT(breastCropProject)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories("$(PROJECT_SOURCE_DIR)")

SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_CXX_FLAGS  "-std=c++0x ${CMAKE_CXX_FLAGS}")

add_executable(breastCrop breastCrop.cxx)

target_link_libraries(breastCrop z lapack blas boost_program_options ${VTK_LIBRARIES})
