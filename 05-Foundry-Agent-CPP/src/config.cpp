#include "foundry_agent/config.hpp"

#include "foundry_agent/errors.hpp"

#include <cstdlib>
#include <string>
#include <utility>

namespace foundry_agent {
namespace {

std::string ReadEnvironment(const char* name)
{
    const char* value = std::getenv(name);
    return value == nullptr ? std::string{} : std::string{value};
}

} // namespace

Config::Config(std::string projectEndpoint, std::string modelDeployment)
    : projectEndpoint_(std::move(projectEndpoint)),
      modelDeployment_(std::move(modelDeployment))
{
    while (!projectEndpoint_.empty() && projectEndpoint_.back() == '/') {
        projectEndpoint_.pop_back();
    }

    if (!projectEndpoint_.starts_with("https://")) {
        throw ConfigError{"FOUNDRY_PROJECT_ENDPOINT must be an HTTPS URL."};
    }
    if (projectEndpoint_.find("/api/projects/") == std::string::npos) {
        throw ConfigError{
            "FOUNDRY_PROJECT_ENDPOINT must include /api/projects/<project-name>."};
    }
    if (projectEndpoint_.find_first_of("?#") != std::string::npos) {
        throw ConfigError{"FOUNDRY_PROJECT_ENDPOINT must not contain a query or fragment."};
    }
    if (modelDeployment_.empty()) {
        throw ConfigError{"AZURE_AI_MODEL_DEPLOYMENT_NAME must not be empty."};
    }
}

Config Config::FromEnvironment()
{
    auto endpoint = ReadEnvironment("FOUNDRY_PROJECT_ENDPOINT");
    auto model = ReadEnvironment("AZURE_AI_MODEL_DEPLOYMENT_NAME");
    if (endpoint.empty()) {
        throw ConfigError{"Set FOUNDRY_PROJECT_ENDPOINT environment variable."};
    }
    if (model.empty()) {
        model = DefaultModel;
    }
    return Config{std::move(endpoint), std::move(model)};
}

const std::string& Config::ProjectEndpoint() const noexcept
{
    return projectEndpoint_;
}

const std::string& Config::ModelDeployment() const noexcept
{
    return modelDeployment_;
}

std::string Config::ResponsesUrl() const
{
    return projectEndpoint_ + "/openai/v1/responses";
}

} // namespace foundry_agent
