# 设置CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(MMU VERSION 1.0)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加一个可执行文件
add_executable(mmu main.cc sim.cc proc.cc pager.cc)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加编译器标志以启用AddressSanitizer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
