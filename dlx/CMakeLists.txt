cmake_minimum_required(VERSION 3.22)

project(dlx_all)
add_subdirectory(dlx_src)
add_subdirectory(dlx_test)

if (ENABLE_TESTING)
    enable_testing()
    message("Building Tests. Be sure to check out test/constexpr_tests for constexpr
testing")
endif ()

