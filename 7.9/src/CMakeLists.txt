cmake_minimum_required(VERSION 3.0)

project(PROJECT2)

find_package(OpenCV REQUIRED)

include_directories(
    ${OpenCV_DIRS}
)

add_executable(PnP_cmake PnP.cpp)

target_link_libraries(PnP_cmake ${OpenCV_LIBS})