set(CMAKE_CXX_STANDARD 20)

find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)

# Generic test that uses conan libs
clangformat_setup(constraints.hpp constraints.cpp)
add_library(dlx_src constraints.cpp
        )
target_link_libraries(dlx_src
        PRIVATE spdlog::spdlog
        )

target_include_directories(dlx_src PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

