cmake_minimum_required(VERSION 3.25)

project(maf_agent_cpp_05 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(azure-core-cpp CONFIG REQUIRED)
find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(foundry_agent STATIC
    src/agent.cpp
    src/config.cpp
    src/http_transport.cpp
)
add_library(FoundryAgent::foundry_agent ALIAS foundry_agent)

target_include_directories(foundry_agent
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(foundry_agent
    PUBLIC
        Azure::azure-core
        Azure::azure-identity
        CURL::libcurl
        nlohmann_json::nlohmann_json
)
target_compile_features(foundry_agent PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(foundry_agent PRIVATE /W4 /permissive-)
else()
    target_compile_options(foundry_agent PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(maf_agent_cpp_05 src/main.cpp)
target_link_libraries(maf_agent_cpp_05 PRIVATE FoundryAgent::foundry_agent)

option(MAF_CPP05_BUILD_TESTS "Build MAF-Agent-CPP-05 tests" ON)
if(MAF_CPP05_BUILD_TESTS)
    include(CTest)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(maf_agent_cpp_05_tests
        tests/agent_tests.cpp
        tests/config_tests.cpp
    )
    target_include_directories(maf_agent_cpp_05_tests PRIVATE tests)
    target_link_libraries(maf_agent_cpp_05_tests
        PRIVATE
            FoundryAgent::foundry_agent
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(maf_agent_cpp_05_tests)
endif()
