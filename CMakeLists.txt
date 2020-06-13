cmake_minimum_required(VERSION 3.0)

project(Test VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -pthread")

add_executable(Test src/main.cpp
                    src/game.cpp
                    src/chesspiece.cpp)
