#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace foundry_agent {

class AgentError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ConfigError final : public AgentError {
public:
    using AgentError::AgentError;
};

class AuthenticationError final : public AgentError {
public:
    using AgentError::AgentError;
};

class TransportError final : public AgentError {
public:
    using AgentError::AgentError;
};

class ResponseError final : public AgentError {
public:
    using AgentError::AgentError;
};

class ServiceError final : public AgentError {
public:
    ServiceError(int statusCode, std::string serviceCode, std::string message)
        : AgentError(std::move(message)),
          statusCode_(statusCode),
          serviceCode_(std::move(serviceCode)) {}

    [[nodiscard]] int StatusCode() const noexcept { return statusCode_; }
    [[nodiscard]] const std::string& ServiceCode() const noexcept { return serviceCode_; }

private:
    int statusCode_;
    std::string serviceCode_;
};

} // namespace foundry_agent
