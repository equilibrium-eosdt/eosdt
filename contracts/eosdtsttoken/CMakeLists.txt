cmake_minimum_required(VERSION 3.5)
project(eosio_token_example VERSION 1.0.0)
find_package(eosio.cdt)

include(../eosdt/CMakeFunc.txt)
git_init(GIT_BRANCH  GIT_COMMIT GIT_VERSION)

get_file_size(ABI_SIZE ${CMAKE_CURRENT_SOURCE_DIR}/eosdtsttoken.abi)
get_file_size(WASM_SIZE ${CMAKE_CURRENT_BINARY_DIR}/eosdtsttoken.wasm)
file(GLOB_RECURSE SOURCE_FILES "*.hpp" "*.cpp")
get_count_line_of_code(CLOC "${SOURCE_FILES}")


configure_file(../eosdt/info.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/info.hpp)

add_contract(eosdtsttoken  eosdtsttoken ${CMAKE_CURRENT_SOURCE_DIR}/src/eosdtsttoken.cpp)

target_include_directories(eosdtsttoken.wasm
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(eosdtsttoken.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR})










