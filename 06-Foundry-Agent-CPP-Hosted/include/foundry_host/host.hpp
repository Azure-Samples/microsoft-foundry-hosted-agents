#pragma once

#include <httplib.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace foundry_host
{

    constexpr int DefaultPort = 8088;
    constexpr std::size_t MaximumRequestSize = 1024 * 1024;

    struct InvocationInput final
    {
        std::string prompt;
        bool isAgUi{};
        std::string threadId;
        std::string runId;
    };

    InvocationInput ParseInvocation(std::string_view body);
    int ResolvePort(const char *value);

    class AgentRunner
    {
    public:
        virtual ~AgentRunner() = default;
        virtual std::string Run(const std::string &prompt) = 0;
    };

    using AgentFactory = std::function<std::shared_ptr<AgentRunner>()>;

    class Host final
    {
    public:
        explicit Host(AgentFactory agentFactory);

        void Configure(httplib::Server &server);
        void HandleReadiness(const httplib::Request &request, httplib::Response &response) const;
        void HandleInvocation(const httplib::Request &request, httplib::Response &response);

    private:
        std::shared_ptr<AgentRunner> GetAgent();

        AgentFactory agentFactory_;
        std::mutex agentMutex_;
        std::shared_ptr<AgentRunner> agent_;
    };

    std::shared_ptr<AgentRunner> CreateDefaultAgentRunner();

} // namespace foundry_host
