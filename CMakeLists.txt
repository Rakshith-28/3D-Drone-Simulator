cmake_minimum_required(VERSION 3.20)
project(DroneNavigation3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    freeglut
    GIT_REPOSITORY https://github.com/freeglut/freeglut.git
    GIT_TAG v3.6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(freeglut)

find_package(OpenGL REQUIRED)

add_executable(drone_simulator src/main.cpp)
set_target_properties(drone_simulator PROPERTIES OUTPUT_NAME drone_simulator_enhanced)
target_include_directories(drone_simulator PRIVATE "${freeglut_SOURCE_DIR}/include")
target_link_libraries(drone_simulator PRIVATE freeglut_static OpenGL::GL OpenGL::GLU)
target_compile_definitions(drone_simulator PRIVATE FREEGLUT_STATIC)

if(MSVC)
    target_compile_options(drone_simulator PRIVATE /W4 /permissive-)
else()
    target_compile_options(drone_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()
