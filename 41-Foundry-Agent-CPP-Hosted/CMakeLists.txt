cmake_minimum_required(VERSION 3.25)

project(maf_agent_cpp_06 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MAF_CPP05_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(../05-Foundry-Agent-CPP MAF-Agent-CPP-05)

find_package(httplib CONFIG REQUIRED)

add_library(foundry_host STATIC
    src/host.cpp
)
target_include_directories(foundry_host
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(foundry_host
    PUBLIC
        FoundryAgent::foundry_agent
        httplib::httplib
)
target_compile_features(foundry_host PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(foundry_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(foundry_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(maf_agent_cpp_06 src/main.cpp)
target_link_libraries(maf_agent_cpp_06 PRIVATE foundry_host)

option(MAF_CPP06_BUILD_TESTS "Build MAF-Agent-CPP-06 tests" ON)
if(MAF_CPP06_BUILD_TESTS)
    include(CTest)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(maf_agent_cpp_06_tests tests/host_tests.cpp)
    target_link_libraries(maf_agent_cpp_06_tests
        PRIVATE
            foundry_host
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(maf_agent_cpp_06_tests)
endif()
