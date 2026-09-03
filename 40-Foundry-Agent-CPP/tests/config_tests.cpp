#include "foundry_agent/config.hpp"
#include "foundry_agent/errors.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Config normalizes a trailing slash")
{
    const foundry_agent::Config config{
        "https://example.services.ai.azure.com/api/projects/sample/",
        "gpt-5-mini"};
    REQUIRE(
        config.ResponsesUrl() ==
        "https://example.services.ai.azure.com/api/projects/sample/openai/v1/responses");
    REQUIRE(config.Instructions() == foundry_agent::Config::DefaultInstructions);
}

TEST_CASE("Config accepts host-specific instructions")
{
    const foundry_agent::Config config{
        "https://example.services.ai.azure.com/api/projects/sample",
        "gpt-5-mini",
        "You are the C++ hosted agent."};

    REQUIRE(config.Instructions() == "You are the C++ hosted agent.");
}

TEST_CASE("Config rejects an insecure endpoint")
{
    REQUIRE_THROWS_AS(
        (foundry_agent::Config{
            "http://example.services.ai.azure.com/api/projects/sample",
            "gpt-5-mini"}),
        foundry_agent::ConfigError);
}

TEST_CASE("Config requires a project path")
{
    REQUIRE_THROWS_AS(
        (foundry_agent::Config{
            "https://example.services.ai.azure.com",
            "gpt-5-mini"}),
        foundry_agent::ConfigError);
}

TEST_CASE("Config rejects empty instructions")
{
    REQUIRE_THROWS_AS(
        (foundry_agent::Config{
            "https://example.services.ai.azure.com/api/projects/sample",
            "gpt-5-mini",
            ""}),
        foundry_agent::ConfigError);
}
