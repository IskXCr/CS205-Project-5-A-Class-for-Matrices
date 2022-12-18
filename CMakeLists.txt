cmake_minimum_required(VERSION 3.22)

# set the project name
project(Project_5_A_Class_for_Matrices)

aux_source_directory(./src DIR_SRCS)
include_directories(./include)

find_package(OpenMP REQUIRED)

# add_compile_options(-O3 -std=c++11)

# add the executable
add_executable(test ${DIR_SRCS})
target_link_libraries(test PRIVATE OpenMP::OpenMP_C)