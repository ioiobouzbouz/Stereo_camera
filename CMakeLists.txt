
cmake_minimum_required(VERSION 3.15)
project(StereoConfig) //Change depending on name of your project

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires a 64-bit build.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/TeliCamSDK/include) // To change
link_directories(${CMAKE_SOURCE_DIR}/TeliCamSDK/lib) // To change

add_executable(StereoConfig main.cpp) 
target_link_libraries(StereoConfig TeliCamApi64 TeliCamUtl64) // To adapt
