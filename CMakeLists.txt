cmake_minimum_required(VERSION 3.27)

project(half_edges VERSION 0.1 LANGUAGES CXX)

option(BUILD_TESTS "Enable testing" ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)


# is no build type is specified, default to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(BUILD_SHARED_LIBS)
    if(WIN32)
        # export all symbols on a windows build of dynamic libs
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS  ON)
    endif()
endif()

set(HE_CXX_STANDARD 23)
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD ${HE_CXX_STANDARD})
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
message(STATUS "Using C++ standard: ${CMAKE_CXX_STANDARD}")

set(HE_CXX_FEATURE cxx_std_${CMAKE_CXX_STANDARD})

# compile options for all targets depending on the compiler and the system
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(MY_COMPILE_OPTIONS "-Wall;-Wextra;-pedantic;-Wno-comment;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;-Wconversion;-Wmisleading-indentation;-Wsign-conversion;-Wfloat-equal")
    # Disable stringop-overflow warning in release builds with GCC as probably the compiler's static analysis
    # is incorrectly flagging a potential buffer overflow during vector reallocation
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            list(APPEND MY_COMPILE_OPTIONS "-Wno-stringop-overflow")
        endif()
    endif()
    if(ENABLE_WARNINGS_AS_ERRORS)
        list(APPEND MY_COMPILE_OPTIONS "-Werror")
    endif()
elseif(MSVC)
    set(MY_COMPILE_OPTIONS "/W4;/permissive-")
    if(ENABLE_WARNINGS_AS_ERRORS)
        list(APPEND MY_COMPILE_OPTIONS "/WX")
    endif()
endif()
if(APPLE)
    list(APPEND MY_COMPILE_OPTIONS "-Wno-deprecated-declarations")
endif()

# compile definitions for all targets depending on the compiler and the system
set(MY_COMPILE_DEFINITIONS "")
if(MSVC)
    set(MY_COMPILE_DEFINITIONS "-DNOMINMAX;-D_USE_MATH_DEFINES")
endif()
if(BUILD_TESTS)
    list(APPEND MY_COMPILE_DEFINITIONS "-DHE_BUILD_TESTS")
endif()

set(LIB_SOURCE_FILES Triangulation.cpp model_io.cpp)

set(LIB_HEADER_FILES Triangulation.hpp model_io.hpp)

set(LIBRARY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(halfedges ${LIB_SOURCE_FILES} ${LIB_HEADER_FILES})
target_include_directories(halfedges PUBLIC $<BUILD_INTERFACE:${LIBRARY_INCLUDE_DIR}>)
target_compile_options(halfedges PRIVATE ${MY_COMPILE_OPTIONS})
target_compile_definitions(halfedges PUBLIC ${MY_COMPILE_DEFINITIONS})
target_compile_features(halfedges PUBLIC ${HE_CXX_FEATURE})

add_executable(main main.cpp)
target_link_libraries(main halfedges)
target_compile_options(main PRIVATE ${MY_COMPILE_OPTIONS})
target_compile_definitions(main PUBLIC ${MY_COMPILE_DEFINITIONS})
target_compile_features(main PUBLIC ${HE_CXX_FEATURE})

if(BUILD_TESTS)
#     find_package(Boost COMPONENTS unit_test_framework REQUIRED)
#     enable_testing()
#     set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#     include(BoostTestHelper)
#     add_boost_test(SOURCE tests/basic_test.cpp LINK halfedges PREFIX half_edges)
# #    add_boost_test(SOURCE src/tests/interpolation_test.cpp LINK curves PREFIX curves)
# #    add_boost_test(SOURCE src/tests/point_test.cpp LINK curves PREFIX curves)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()