cmake_minimum_required(VERSION 3.2)
project(DirectX12Example)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 23)

# Add executable target with source files
add_executable(DirectX12Example main.cpp)

# Link against the DirectX 12 libraries
target_link_libraries(DirectX12Example d3d12.lib dxgi.lib)
