#pragma once

#include "foundry_agent/config.hpp"
#include "foundry_agent/http_transport.hpp"

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>

#include <memory>
#include <string>

namespace foundry_agent {

struct AgentResponse final {
    std::string text;
    std::string responseId;
};

class Agent final {
public:
    Agent(
        Config config,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::shared_ptr<HttpTransport> transport = {});

    static Agent CreateDefault(Config config);

    AgentResponse Run(
        const std::string& input,
        const Azure::Core::Context& context = {}) const;

private:
    Config config_;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential_;
    std::shared_ptr<HttpTransport> transport_;
};

} // namespace foundry_agent
