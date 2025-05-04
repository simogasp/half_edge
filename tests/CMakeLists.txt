enable_testing()
find_package(Catch2 3.8 QUIET)
if (NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, downloading...")
    include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.1
    )
    FetchContent_MakeAvailable(Catch2)
endif ()

add_executable(basic_test basic_test.cpp)
target_link_libraries(basic_test PRIVATE Catch2::Catch2WithMain halfedges)


list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(basic_test)

add_test(NAME he_basic_test COMMAND basic_test)
