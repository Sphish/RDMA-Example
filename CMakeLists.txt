cmake_minimum_required (VERSION 3.11)
project(RDMA-Example)

add_executable(example1 example1.cpp ib.cpp)
target_link_libraries(example1 ibverbs)