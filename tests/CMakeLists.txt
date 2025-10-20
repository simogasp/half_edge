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

add_executable(model_io_test model_io_test.cpp)
target_link_libraries(model_io_test PRIVATE Catch2::Catch2WithMain halfedges)
target_compile_options(model_io_test PRIVATE ${MY_COMPILE_OPTIONS})
target_compile_definitions(model_io_test PUBLIC ${MY_COMPILE_DEFINITIONS})
target_compile_features(model_io_test PUBLIC ${HE_CXX_FEATURE})

# For Windows: Copy Catch2 DLL next to the test executable
if(WIN32)
    add_custom_command(TARGET model_io_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Catch2::Catch2>
        $<TARGET_FILE_DIR:model_io_test>
    )
    add_custom_command(TARGET model_io_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Catch2::Catch2WithMain>
        $<TARGET_FILE_DIR:model_io_test>
    )
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(model_io_test DISCOVERY_MODE PRE_TEST)

add_test(NAME he_model_io_test COMMAND $<TARGET_FILE:model_io_test>)