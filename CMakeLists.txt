cmake_minimum_required(VERSION 2.8)
project(DistributedSquareCounting)

include_directories(
        ${PROJECT_SOURCE_DIR}/
)

set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -O3 -g -Wall -march=native -pthread")

set(DISTRIBUTED_SQUARE_COUNTING_FILE_LIST
        commandparser.h
        commandparser.cpp
        constants.h
        countingalgo.h
        countingalgo.cpp
        graph.cpp
        graph.h
        loadbalancer.h
        loadbalancer.cpp
        streaminggraph.h
        streaminggraph.cpp
        types.h
        StudyPerformance.cpp)


add_executable(SquareCounting.out ${DISTRIBUTED_SQUARE_COUNTING_FILE_LIST})

find_package(MPI REQUIRED)
target_link_libraries(SquareCounting.out PUBLIC MPI::MPI_CXX)