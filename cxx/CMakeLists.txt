cmake_minimum_required (VERSION 3.12)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project (CategorialQueryIndex)
set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -march=native -fopenmp -D_GLIBCXX_PARALLEL -fext-numeric-literals -pthread")
# Suppress warnings. Replace with previous line to enable warnings.
set(CMAKE_CXX_FLAGS "-fmax-errors=10 -std=c++17 -march=native -fopenmp -D_GLIBCXX_PARALLEL -fext-numeric-literals -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mfma -mbmi2 -ffast-math -fno-tree-vectorize -fno-finite-math-only")
# Flags for the Park RL environment.
set(CMAKE_PARK_FLAGS "-march=x86-64")
option(NDEBUG "Turn off logging" OFF)
set (TESTDIR "tests")

if (NOT DIM)
    add_definitions(-DDIM=2)
endif()

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
add_executable(run_mapped_correlation_index run_correlation_index.cpp ${SOURCES})
add_executable(run_linear_correlation_index run_correlation_index.cpp ${SOURCES})
target_compile_definitions(run_linear_correlation_index PUBLIC USE_LINEAR=1)


configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

add_executable(test_primary_btree_index ${TESTDIR}/test_primary_btree_index.cpp ${SOURCES})
target_link_libraries(test_primary_btree_index gtest_main)
add_executable(test_binary_search_index ${TESTDIR}/test_binary_search_index.cpp ${SOURCES})
target_link_libraries(test_binary_search_index gtest_main)
add_executable(test_secondary_btree_index ${TESTDIR}/test_secondary_btree_index.cpp ${SOURCES})
target_link_libraries(test_secondary_btree_index gtest_main)
add_executable(test_outlier_index ${TESTDIR}/test_outlier_index.cpp ${SOURCES})
target_link_libraries(test_outlier_index gtest_main)
add_executable(test_composite_index ${TESTDIR}/test_composite_index.cpp ${SOURCES})
target_link_libraries(test_composite_index gtest_main)
add_executable(test_single_column_rewriter ${TESTDIR}/test_single_column_rewriter.cpp ${SOURCES})
target_link_libraries(test_single_column_rewriter gtest_main)

