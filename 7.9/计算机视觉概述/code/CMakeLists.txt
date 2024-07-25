cmake_minimum_required(VERSION 3.15)

project(cv)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
)

add_executable(split split.cpp)
add_executable(hsv hsv.cpp)
add_executable(filter filter.cpp)
add_executable(fourier fourier.cpp)
add_executable(sobel sobel.cpp)
add_executable(canny canny.cpp)
add_executable(corner corner.cpp)
add_executable(morphology morphology.cpp)

target_link_libraries(split
    ${OpenCV_LIBS}
)
target_link_libraries(hsv
    ${OpenCV_LIBS}
)
target_link_libraries(filter
    ${OpenCV_LIBS}
)
target_link_libraries(fourier
    ${OpenCV_LIBS}
)
target_link_libraries(sobel
    ${OpenCV_LIBS}
)
target_link_libraries(canny
    ${OpenCV_LIBS}
)
target_link_libraries(corner
    ${OpenCV_LIBS}
)
target_link_libraries(morphology
    ${OpenCV_LIBS}
)