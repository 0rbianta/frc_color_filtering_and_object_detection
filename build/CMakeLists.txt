cmake_minimum_required(VERSION 2.8)
project(orbianta_frc_color_filtering)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(PRJ_ROOT ../src)
add_executable(main ${PRJ_ROOT}/main.cpp ${PRJ_ROOT}/fs.cpp)

target_link_libraries(main ${OpenCV_LIBS})

