#pragma once

#include "foundry_agent/http_transport.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/datetime.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <utility>

namespace foundry_agent::tests
{

    class FakeCredential final : public Azure::Core::Credentials::TokenCredential
    {
    public:
        FakeCredential() : TokenCredential("FakeCredential") {}

        Azure::Core::Credentials::AccessToken GetToken(
            const Azure::Core::Credentials::TokenRequestContext &request,
            const Azure::Core::Context &) const override
        {
            requestedScope = request.Scopes.empty() ? "" : request.Scopes.front();
            return {
                "test-token",
                Azure::DateTime{
                    std::chrono::system_clock::now() + std::chrono::hours{1}}};
        }

        mutable std::string requestedScope;
    };

    class FakeTransport final : public HttpTransport
    {
    public:
        HttpResponse Post(const HttpRequest &request) override
        {
            lastRequest = request;
            return response;
        }

        HttpRequest lastRequest;
        HttpResponse response;
    };

} // namespace foundry_agent::tests
