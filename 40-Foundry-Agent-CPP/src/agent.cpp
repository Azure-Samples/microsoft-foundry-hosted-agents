#include "foundry_agent/agent.hpp"

#include "foundry_agent/errors.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/identity/default_azure_credential.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <utility>

namespace foundry_agent
{
    namespace
    {

        std::string ExtractErrorCode(const nlohmann::json &document)
        {
            if (document.contains("error") && document["error"].is_object())
            {
                return document["error"].value("code", "");
            }
            return {};
        }

        std::string ExtractErrorMessage(const nlohmann::json &document)
        {
            if (document.contains("error") && document["error"].is_object())
            {
                return document["error"].value("message", "Foundry returned an error.");
            }
            return "Foundry returned an error.";
        }

    } // namespace

    Agent::Agent(
        Config config,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::shared_ptr<HttpTransport> transport)
        : config_(std::move(config)),
          credential_(std::move(credential)),
          transport_(transport ? std::move(transport) : MakeCurlTransport())
    {
        if (!credential_)
        {
            throw ConfigError{"A token credential is required."};
        }
    }

    Agent Agent::CreateDefault(Config config)
    {
        return Agent{
            std::move(config),
            std::make_shared<Azure::Identity::DefaultAzureCredential>()};
    }

    AgentResponse Agent::Run(
        const std::string &input,
        const Azure::Core::Context &context) const
    {
        if (input.empty())
        {
            throw ResponseError{"Agent input must not be empty."};
        }

        Azure::Core::Credentials::TokenRequestContext tokenRequest;
        tokenRequest.Scopes = {std::string{Config::TokenScope}};

        std::string token;
        try
        {
            token = credential_->GetToken(tokenRequest, context).Token;
        }
        catch (const Azure::Core::Credentials::AuthenticationException &error)
        {
            throw AuthenticationError{"Failed to acquire a Foundry access token: " +
                                      std::string{error.what()}};
        }

        const nlohmann::json requestDocument{
            {"model", config_.ModelDeployment()},
            {"instructions", config_.Instructions()},
            {"input", nlohmann::json::array({{{"role", "user"},
                                              {"content", input}}})}};

        const HttpResponse response = transport_->Post(HttpRequest{
            config_.ResponsesUrl(),
            {{"Accept", "application/json"},
             {"Authorization", "Bearer " + token},
             {"Content-Type", "application/json"}},
            requestDocument.dump()});

        nlohmann::json responseDocument;
        try
        {
            responseDocument = nlohmann::json::parse(response.body);
        }
        catch (const nlohmann::json::exception &)
        {
            if (response.statusCode < 200 || response.statusCode >= 300)
            {
                throw ServiceError{
                    response.statusCode, "", "Foundry returned a non-JSON error response."};
            }
            throw ResponseError{"Foundry returned malformed JSON."};
        }

        if (response.statusCode < 200 || response.statusCode >= 300)
        {
            throw ServiceError{
                response.statusCode,
                ExtractErrorCode(responseDocument),
                ExtractErrorMessage(responseDocument)};
        }

        AgentResponse result;
        result.responseId = responseDocument.value("id", "");

        if (responseDocument.contains("output") && responseDocument["output"].is_array())
        {
            for (const auto &output : responseDocument["output"])
            {
                if (!output.is_object() || !output.contains("content") ||
                    !output["content"].is_array())
                {
                    continue;
                }
                for (const auto &content : output["content"])
                {
                    if (content.is_object() && content.value("type", "") == "output_text")
                    {
                        if (!result.text.empty())
                        {
                            result.text.push_back('\n');
                        }
                        result.text += content.value("text", "");
                    }
                }
            }
        }

        if (result.text.empty())
        {
            throw ResponseError{"Foundry response did not contain assistant output text."};
        }
        return result;
    }

} // namespace foundry_agent
