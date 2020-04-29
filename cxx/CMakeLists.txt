cmake_minimum_required (VERSION 3.12)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project (CategorialQueryIndex)
set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -march=native -fopenmp -D_GLIBCXX_PARALLEL -fext-numeric-literals -pthread")
# Suppress warnings. Replace with previous line to enable warnings.
set(CMAKE_CXX_FLAGS "-std=c++17 -march=native -fopenmp -D_GLIBCXX_PARALLEL -fext-numeric-literals -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mfma -mbmi2 -ffast-math -fno-tree-vectorize -fno-finite-math-only")
# Flags for the Park RL environment.
set(CMAKE_PARK_FLAGS "-march=x86-64")
option(NDEBUG "Turn off logging" OFF)

add_definitions(-DDIM=${DIM})
if (NDEBUG)
    # Add -DNDEBUG=ON as an argument to cmake to turn off debugging.
    # To turn it off, use -DNDEBUG=OFF
    add_definitions(-D NDEBUG=1)
    message("Turning NDEBUG ON")
endif(NDEBUG)

#find_library(LIBSPAT spatialindex)
#set(BOOST_ROOT "~" CACHE PATH "Boost library path")
#find_package(Boost COMPONENTS system)
#include_directories(${Boost_INCLUDE_DIRS})
#link_libraries(${Boost_LIBRARIES})

include_directories("include")
include_directories(/usr/local/include)
file(GLOB SOURCES "src/*.cpp")
add_executable(benchmark_categorical_index benchmark_categorical_index.cpp ${SOURCES})

