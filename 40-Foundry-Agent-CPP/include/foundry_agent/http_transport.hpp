#pragma once

#include <map>
#include <memory>
#include <string>

namespace foundry_agent {

struct HttpRequest final {
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse final {
    int statusCode{};
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpTransport {
public:
    virtual ~HttpTransport() = default;
    virtual HttpResponse Post(const HttpRequest& request) = 0;
};

std::shared_ptr<HttpTransport> MakeCurlTransport();

} // namespace foundry_agent
