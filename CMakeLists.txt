CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(YUVPlayer)

INCLUDE_DIRECTORIES(./inc)
ADD_EXECUTABLE(YUVPlayer ./src/YUV.cpp ./src/YUVPlayer.cpp)
TARGET_LINK_LIBRARIES(YUVPlayer GL glut boost_program_options boost_system gomp)

ADD_DEFINITIONS(-O3 -g -fopenmp -fsigned-char -freg-struct-return -Wall)


