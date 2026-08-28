#include "foundry_host/host.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace {

class FakeAgent final : public foundry_host::AgentRunner {
public:
    std::string Run(const std::string& prompt) override
    {
        lastPrompt = prompt;
        return response;
    }

    std::string lastPrompt;
    std::string response{"assistant response"};
};

} // namespace

TEST_CASE("Invocation parser accepts plain text and JSON strings")
{
    REQUIRE(foundry_host::ParseInvocation("  hello  ").prompt == "hello");
    REQUIRE(foundry_host::ParseInvocation(R"("hello")").prompt == "hello");
}

TEST_CASE("Invocation parser recognizes AG-UI and rejects unsupported JSON")
{
    REQUIRE(foundry_host::ParseInvocation(
        R"({"messages":[{"role":"user","content":"hello"}]})").isAgUi);
    REQUIRE_THROWS_AS(foundry_host::ParseInvocation(R"({"messages":[]})"),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(foundry_host::ParseInvocation(R"({"prompt":"hello"})"),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(foundry_host::ParseInvocation("[]"), std::invalid_argument);
    REQUIRE_THROWS_AS(foundry_host::ParseInvocation("  "), std::invalid_argument);
}

TEST_CASE("Port validation honors defaults and valid values")
{
    REQUIRE(foundry_host::ResolvePort(nullptr) == 8088);
    REQUIRE(foundry_host::ResolvePort("") == 8088);
    REQUIRE(foundry_host::ResolvePort("9090") == 9090);
    REQUIRE_THROWS_AS(foundry_host::ResolvePort("0"), std::invalid_argument);
    REQUIRE_THROWS_AS(foundry_host::ResolvePort("invalid"), std::invalid_argument);
}

TEST_CASE("Readiness returns the hosted health contract")
{
    foundry_host::Host host{[] { return std::make_shared<FakeAgent>(); }};
    httplib::Request request;
    request.method = "GET";
    httplib::Response response;

    host.HandleReadiness(request, response);

    REQUIRE(response.status == 200);
    REQUIRE(response.body == R"({"status":"ready"})");
    REQUIRE(response.get_header_value("Content-Type") == "application/json");
}

TEST_CASE("Invocation runs through a lazily initialized stateless agent")
{
    auto agent = std::make_shared<FakeAgent>();
    int factoryCalls = 0;
    foundry_host::Host host{[&] {
        ++factoryCalls;
        return agent;
    }};
    httplib::Request request;
    request.method = "POST";
    request.body = R"("hello")";
    httplib::Response response;

    host.HandleInvocation(request, response);
    REQUIRE(response.status == 200);
    REQUIRE(response.body == "assistant response");
    REQUIRE(agent->lastPrompt == "hello");

    httplib::Response secondResponse;
    host.HandleInvocation(request, secondResponse);
    REQUIRE(factoryCalls == 1);
}

TEST_CASE("AG-UI objects return not implemented")
{
    foundry_host::Host host{[] { return std::make_shared<FakeAgent>(); }};
    httplib::Request request;
    request.method = "POST";
    request.body = R"({"messages":[{"role":"user","content":"hello"}]})";
    httplib::Response response;

    host.HandleInvocation(request, response);
    REQUIRE(response.status == 501);
}

TEST_CASE("Oversized requests return payload too large")
{
    foundry_host::Host host{[] { return std::make_shared<FakeAgent>(); }};
    httplib::Request request;
    request.method = "POST";
    request.body.assign(foundry_host::MaximumRequestSize + 1, 'x');
    httplib::Response response;

    host.HandleInvocation(request, response);
    REQUIRE(response.status == 413);
}
