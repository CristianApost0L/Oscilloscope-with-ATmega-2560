# TODO compile executable that will be run  
cmake_minimum_required(VERSION 2.8.3)
project(mini_core)

add_executable(mini_core many_object_in_stack_example.cpp)

target_link_libraries(mini_core core_library)
