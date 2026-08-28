#include "foundry_host/host.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace
{

    class FakeAgent final : public foundry_host::AgentRunner
    {
    public:
        std::string Run(const std::string &prompt) override
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
    const auto invocation = foundry_host::ParseInvocation(
        R"({"threadId":"thread-1","runId":"run-1","messages":[{"role":"assistant","content":"How can I help?"},{"role":"user","content":"hello"}]})");
    REQUIRE(invocation.isAgUi);
    REQUIRE(invocation.prompt == "hello");
    REQUIRE(invocation.threadId == "thread-1");
    REQUIRE(invocation.runId == "run-1");

    REQUIRE_THROWS_AS(foundry_host::ParseInvocation(R"({"messages":[]})"),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(foundry_host::ParseInvocation(
                          R"({"threadId":"thread-1","runId":"run-1","messages":[{"role":"assistant","content":"hello"}]})"),
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
    foundry_host::Host host{[]
                            { return std::make_shared<FakeAgent>(); }};
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
    foundry_host::Host host{[&]
                            {
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

TEST_CASE("AG-UI objects return Inspector-compatible server-sent events")
{
    auto agent = std::make_shared<FakeAgent>();
    agent->response = "Hello from C++.";
    foundry_host::Host host{[agent]
                            { return agent; }};
    httplib::Request request;
    request.method = "POST";
    request.body = R"({"threadId":"thread-1","runId":"run-1","messages":[{"id":"message-1","role":"user","content":"hello"}]})";
    httplib::Response response;

    host.HandleInvocation(request, response);

    REQUIRE(response.status == 200);
    REQUIRE(response.get_header_value("Content-Type") == "text/event-stream");
    REQUIRE(response.get_header_value("Cache-Control") == "no-cache");
    REQUIRE(agent->lastPrompt == "hello");

    const auto runStarted = response.body.find(R"("type":"RUN_STARTED")");
    const auto messageStarted = response.body.find(R"("type":"TEXT_MESSAGE_START")");
    const auto content = response.body.find(R"("type":"TEXT_MESSAGE_CONTENT")");
    const auto messageEnded = response.body.find(R"("type":"TEXT_MESSAGE_END")");
    const auto runFinished = response.body.find(R"("type":"RUN_FINISHED")");
    REQUIRE(runStarted < messageStarted);
    REQUIRE(messageStarted < content);
    REQUIRE(content < messageEnded);
    REQUIRE(messageEnded < runFinished);
    REQUIRE(response.body.find(R"("threadId":"thread-1")") != std::string::npos);
    REQUIRE(response.body.find(R"("runId":"run-1")") != std::string::npos);
    REQUIRE(response.body.find(R"("delta":"Hello from C++.")") != std::string::npos);
    REQUIRE(response.body.find(R"("timestamp":)") != std::string::npos);
}

TEST_CASE("Oversized requests return payload too large")
{
    foundry_host::Host host{[]
                            { return std::make_shared<FakeAgent>(); }};
    httplib::Request request;
    request.method = "POST";
    request.body.assign(foundry_host::MaximumRequestSize + 1, 'x');
    httplib::Response response;

    host.HandleInvocation(request, response);
    REQUIRE(response.status == 413);
}
