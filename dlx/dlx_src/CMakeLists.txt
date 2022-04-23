find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)

# Generic test that uses conan libs
clangformat_setup(dlx_main.cpp)
add_executable(dlx dlx_main.cpp)
target_link_libraries(
        dlx
        PRIVATE project_options
        project_warnings
        docopt::docopt
        fmt::fmt
        spdlog::spdlog)

target_include_directories(dlx PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

