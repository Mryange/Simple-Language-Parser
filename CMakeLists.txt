
cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/..)
# 项目信息
project (Demo)

# 指定生成目标
aux_source_directory(. DIR_SRCS)
add_executable(Demo ${DIR_SRCS})


# 添加 math 子目录
add_subdirectory(src)

# 添加链接库
target_link_libraries(Demo src)