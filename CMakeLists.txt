cmake_minimum_required(VERSION 3.15)
project(ImGui_DX12_Example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")


# Specify the paths to the necessary ImGui files and backends
set(IMGUI_PATH "C:/libs/imgui")

set(SOURCES 
    main.cpp
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/backends/imgui_impl_dx12.cpp
    ${IMGUI_PATH}/backends/imgui_impl_win32.cpp
    hardware/webcam/webcam_manager.cpp
    hardware/webcam/webcam.cpp 
    hardware/webcam/GUID_tools.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include ImGui directory
include_directories(${IMGUI_PATH})
include_directories(${IMGUI_PATH}/backends)

# Link against DirectX 12 libraries
target_link_libraries(${PROJECT_NAME} 
                      d3d12 
                      dxgi
                      MFplat.lib 
                      MF.lib 
                      Mfreadwrite.lib 
                      Mfuuid.lib 
                      ${AVCODEC_LIBRARY} 
                      ${AVFORMAT_LIBRARY} 
                      ${AVUTIL_LIBRARY} 
                      ${SWSCALE_LIBRARY}
                      uuid) 

# Define preprocessor directives for enabling the debug layer in debug builds
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DX12_ENABLE_DEBUG_LAYER>")

# Set runtime library linkage dynamically (for Visual Studio)
foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
   if(${flag_var} MATCHES "/MT")
      string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MT")
endforeach()

# Specify Unicode for Windows targets
target_compile_definitions(${PROJECT_NAME} PRIVATE "_UNICODE" "UNICODE")

# For Visual Studio, set the working directory to the source directory
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# ----------------- Webcam ask -----------------

add_executable(Webcamask testcam.cpp)

target_link_libraries(Webcamask MFplat.lib MF.lib Mfreadwrite.lib Mfuuid.lib)
