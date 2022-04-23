project(myproject_test)

find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

add_library(catch_main OBJECT catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE project_options)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME dlx.has_help COMMAND dlx --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME dlx.version_matches COMMAND dlx --version)
set_tests_properties(dlx.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")


clangformat_setup(tests.cpp)
add_executable(dlx_tests tests.cpp)
target_link_libraries(dlx_tests PRIVATE project_warnings project_options catch_main)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
        dlx_tests
        TEST_PREFIX
        "unittests."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittests."
        OUTPUT_SUFFIX
        .xml)

# Add a file containing a set of constexpr tests
add_executable(dlx_constexpr_tests constexpr_tests.cpp)
target_link_libraries(dlx_constexpr_tests PRIVATE project_options project_warnings catch_main)

catch_discover_tests(
        dlx_constexpr_tests
        TEST_PREFIX
        "constexpr."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "dlx_constexpr."
        OUTPUT_SUFFIX
        .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
add_executable(dlx_relaxed_constexpr_tests constexpr_tests.cpp)
target_link_libraries(dlx_relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main)
target_compile_definitions(dlx_relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

catch_discover_tests(
        dlx_relaxed_constexpr_tests
        TEST_PREFIX
        "relaxed_constexpr."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "dlx_relaxed_constexpr."
        OUTPUT_SUFFIX
        .xml)
