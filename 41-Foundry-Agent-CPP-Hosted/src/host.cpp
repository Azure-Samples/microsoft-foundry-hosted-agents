#include "foundry_host/host.hpp"

#include "foundry_agent/agent.hpp"
#include "foundry_agent/config.hpp"
#include "foundry_agent/errors.hpp"

#include <nlohmann/json.hpp>

#include <atomic>
#include <charconv>
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

namespace foundry_host
{
    namespace
    {

        std::string_view Trim(std::string_view value)
        {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string_view::npos)
            {
                return {};
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }

        class DefaultAgentRunner final : public AgentRunner
        {
        public:
            DefaultAgentRunner()
                : agent_(foundry_agent::Agent::CreateDefault(
                      foundry_agent::Config::FromEnvironment(
                          "You are the C++ hosted agent sample, running on C++20 with a "
                          "repository-owned Microsoft Foundry hosting adapter. When greeted "
                          "or asked who you are, identify yourself as the C++ hosted agent. Keep "
                          "your answers brief.")))
            {
            }

            std::string Run(const std::string &prompt) override
            {
                return agent_.Run(prompt).text;
            }

        private:
            foundry_agent::Agent agent_;
        };

        void SetText(httplib::Response &response, int status, std::string content)
        {
            response.status = status;
            response.set_content(std::move(content), "text/plain; charset=utf-8");
        }

        std::string RequiredString(
            const nlohmann::json &document,
            std::string_view field,
            std::string_view description)
        {
            const auto iterator = document.find(field);
            if (iterator == document.end() || !iterator->is_string())
            {
                throw std::invalid_argument{std::string{description} + " must be a string."};
            }

            const auto value = Trim(iterator->get_ref<const std::string &>());
            if (value.empty())
            {
                throw std::invalid_argument{std::string{description} + " must not be empty."};
            }
            return std::string{value};
        }

        std::string CreateMessageId()
        {
            static std::atomic_uint64_t nextId{1};
            return "message-" + std::to_string(nextId.fetch_add(1, std::memory_order_relaxed));
        }

        void AppendSseEvent(std::string &stream, const nlohmann::json &event)
        {
            auto timestampedEvent = event;
            timestampedEvent["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                std::chrono::system_clock::now().time_since_epoch())
                                                .count();
            stream += "data: ";
            stream += timestampedEvent.dump();
            stream += "\n\n";
        }

        void SetAgUiResponse(
            httplib::Response &response,
            const InvocationInput &invocation,
            const std::string &output)
        {
            const auto messageId = CreateMessageId();
            std::string stream;
            AppendSseEvent(stream, {
                                       {"type", "RUN_STARTED"},
                                       {"threadId", invocation.threadId},
                                       {"runId", invocation.runId},
                                   });
            AppendSseEvent(stream, {
                                       {"type", "TEXT_MESSAGE_START"},
                                       {"messageId", messageId},
                                       {"role", "assistant"},
                                   });
            AppendSseEvent(stream, {
                                       {"type", "TEXT_MESSAGE_CONTENT"},
                                       {"messageId", messageId},
                                       {"delta", output},
                                   });
            AppendSseEvent(stream, {
                                       {"type", "TEXT_MESSAGE_END"},
                                       {"messageId", messageId},
                                   });
            AppendSseEvent(stream, {
                                       {"type", "RUN_FINISHED"},
                                       {"threadId", invocation.threadId},
                                       {"runId", invocation.runId},
                                   });

            response.set_header("Cache-Control", "no-cache");
            response.set_header("Connection", "keep-alive");
            response.status = 200;
            response.set_content(std::move(stream), "text/event-stream");
        }

    } // namespace

    InvocationInput ParseInvocation(std::string_view body)
    {
        const auto trimmed = Trim(body);
        if (trimmed.empty())
        {
            throw std::invalid_argument{"Request body must not be empty."};
        }

        if (trimmed.front() == '"' || trimmed.front() == '{' || trimmed.front() == '[')
        {
            nlohmann::json document;
            try
            {
                document = nlohmann::json::parse(trimmed);
            }
            catch (const nlohmann::json::exception &)
            {
                throw std::invalid_argument{"Request body contains malformed JSON."};
            }

            if (document.is_string())
            {
                const auto prompt = Trim(document.get<std::string>());
                if (prompt.empty())
                {
                    throw std::invalid_argument{"Prompt must not be empty."};
                }
                return {std::string{prompt}, false, {}, {}};
            }

            if (document.is_object() &&
                document.contains("messages") &&
                document["messages"].is_array() &&
                !document["messages"].empty())
            {
                const auto &messages = document["messages"];
                for (auto iterator = messages.rbegin(); iterator != messages.rend(); ++iterator)
                {
                    if (!iterator->is_object() || iterator->value("role", "") != "user")
                    {
                        continue;
                    }

                    const auto prompt = RequiredString(*iterator, "content", "User message content");
                    return {
                        prompt,
                        true,
                        RequiredString(document, "threadId", "threadId"),
                        RequiredString(document, "runId", "runId"),
                    };
                }

                throw std::invalid_argument{
                    "AG-UI messages must contain a user message with string content."};
            }

            throw std::invalid_argument{
                "JSON invocations must be a string or an AG-UI object containing messages."};
        }

        return {std::string{trimmed}, false, {}, {}};
    }

    int ResolvePort(const char *value)
    {
        if (value == nullptr || *value == '\0')
        {
            return DefaultPort;
        }

        int port = 0;
        const std::string_view text{value};
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), port);
        if (error != std::errc{} || end != text.data() + text.size() ||
            port < 1 || port > 65535)
        {
            throw std::invalid_argument{"PORT must be an integer between 1 and 65535."};
        }
        return port;
    }

    Host::Host(AgentFactory agentFactory)
        : agentFactory_(std::move(agentFactory))
    {
        if (!agentFactory_)
        {
            throw std::invalid_argument{"An agent factory is required."};
        }
    }

    void Host::Configure(httplib::Server &server)
    {
        server.set_payload_max_length(MaximumRequestSize);
        server.set_read_timeout(5, 0);
        server.set_write_timeout(30, 0);
        server.Get("/readiness", [this](const auto &request, auto &response)
                   { HandleReadiness(request, response); });
        server.Post("/invocations", [this](const auto &request, auto &response)
                    { HandleInvocation(request, response); });
    }

    void Host::HandleReadiness(
        const httplib::Request &request,
        httplib::Response &response) const
    {
        if (request.method != "GET")
        {
            response.set_header("Allow", "GET");
            SetText(response, 405, "Method Not Allowed");
            return;
        }
        response.set_header("Cache-Control", "no-store");
        response.status = 200;
        response.set_content(R"({"status":"ready"})", "application/json");
    }

    void Host::HandleInvocation(
        const httplib::Request &request,
        httplib::Response &response)
    {
        if (request.method != "POST")
        {
            response.set_header("Allow", "POST");
            SetText(response, 405, "Method Not Allowed");
            return;
        }
        if (request.body.size() > MaximumRequestSize)
        {
            SetText(response, 413, "Request body exceeds the 1 MiB limit.");
            return;
        }

        InvocationInput invocation;
        try
        {
            invocation = ParseInvocation(request.body);
        }
        catch (const std::invalid_argument &error)
        {
            SetText(response, 400, error.what());
            return;
        }

        try
        {
            auto agent = GetAgent();
            const auto output = agent->Run(invocation.prompt);
            if (invocation.isAgUi)
            {
                SetAgUiResponse(response, invocation, output);
            }
            else
            {
                response.set_header("Cache-Control", "no-store");
                SetText(response, 200, output);
            }
        }
        catch (const foundry_agent::AuthenticationError &error)
        {
            SetText(response, 503, std::string{"Authentication failed: "} + error.what());
        }
        catch (const foundry_agent::ServiceError &error)
        {
            SetText(
                response,
                error.StatusCode() == 429 ? 503 : 502,
                std::string{"Foundry request failed: "} + error.what());
        }
        catch (const foundry_agent::AgentError &error)
        {
            SetText(response, 502, std::string{"Foundry request failed: "} + error.what());
        }
        catch (const std::exception &error)
        {
            SetText(response, 503, std::string{"Agent initialization failed: "} + error.what());
        }
    }

    std::shared_ptr<AgentRunner> Host::GetAgent()
    {
        std::lock_guard lock{agentMutex_};
        if (!agent_)
        {
            agent_ = agentFactory_();
            if (!agent_)
            {
                throw std::runtime_error{"Agent factory returned no agent."};
            }
        }
        return agent_;
    }

    std::shared_ptr<AgentRunner> CreateDefaultAgentRunner()
    {
        return std::make_shared<DefaultAgentRunner>();
    }

} // namespace foundry_host
