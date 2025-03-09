# cmake_minimum_required(VERSION 3.0)
# project(main)
#可以结合大文件夹


# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置cmake的全局变量
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#添加头文件路径，相当于makefile里面的-I
#include_directories(${PROJECT_SOURCE_DIR}/include)



set(SRC_LIST  ./muduo_server.cpp)
#aux_source_directory(. SRC_LIST)

add_executable(server ${SRC_LIST})

target_link_libraries(server muduo_net muduo_base pthread)