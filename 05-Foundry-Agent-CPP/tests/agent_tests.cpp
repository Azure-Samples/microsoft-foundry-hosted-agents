#include "foundry_agent/agent.hpp"
#include "foundry_agent/errors.hpp"
#include "fake_transport.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <memory>

namespace {

foundry_agent::Config TestConfig()
{
    return {
        "https://example.services.ai.azure.com/api/projects/sample",
        "gpt-5-mini"};
}

} // namespace

TEST_CASE("Agent sends the expected Foundry request")
{
    auto credential = std::make_shared<foundry_agent::tests::FakeCredential>();
    auto transport = std::make_shared<foundry_agent::tests::FakeTransport>();
    transport->response = {
        200,
        {},
        R"({"id":"resp-1","output":[{"type":"message","content":[{"type":"output_text","text":"Hello!"}]}]})"};

    const foundry_agent::Agent agent{TestConfig(), credential, transport};
    const auto response = agent.Run("Hi");

    REQUIRE(response.text == "Hello!");
    REQUIRE(response.responseId == "resp-1");
    REQUIRE(credential->requestedScope == "https://ai.azure.com/.default");
    REQUIRE(
        transport->lastRequest.url ==
        "https://example.services.ai.azure.com/api/projects/sample/openai/v1/responses");
    REQUIRE(transport->lastRequest.headers.at("Authorization") == "Bearer test-token");

    const auto body = nlohmann::json::parse(transport->lastRequest.body);
    REQUIRE(body.at("model") == "gpt-5-mini");
    REQUIRE(body.at("input").at(0).at("content") == "Hi");
}

TEST_CASE("Agent surfaces service errors")
{
    auto credential = std::make_shared<foundry_agent::tests::FakeCredential>();
    auto transport = std::make_shared<foundry_agent::tests::FakeTransport>();
    transport->response = {
        429,
        {},
        R"({"error":{"code":"TooManyRequests","message":"Slow down."}})"};

    const foundry_agent::Agent agent{TestConfig(), credential, transport};
    try {
        (void)agent.Run("Hi");
        FAIL("Expected a service error.");
    } catch (const foundry_agent::ServiceError& error) {
        REQUIRE(error.StatusCode() == 429);
        REQUIRE(error.ServiceCode() == "TooManyRequests");
    }
}

TEST_CASE("Agent rejects responses without output text")
{
    auto credential = std::make_shared<foundry_agent::tests::FakeCredential>();
    auto transport = std::make_shared<foundry_agent::tests::FakeTransport>();
    transport->response = {200, {}, R"({"id":"resp-1","output":[]})"};

    const foundry_agent::Agent agent{TestConfig(), credential, transport};
    REQUIRE_THROWS_AS(agent.Run("Hi"), foundry_agent::ResponseError);
}
