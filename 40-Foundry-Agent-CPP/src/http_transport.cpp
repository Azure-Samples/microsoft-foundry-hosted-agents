#include "foundry_agent/http_transport.hpp"

#include "foundry_agent/errors.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

namespace foundry_agent {
namespace {

size_t WriteBody(char* data, size_t size, size_t count, void* userData)
{
    const size_t length = size * count;
    static_cast<std::string*>(userData)->append(data, length);
    return length;
}

size_t WriteHeader(char* data, size_t size, size_t count, void* userData)
{
    const size_t length = size * count;
    std::string line{data, length};
    const auto separator = line.find(':');
    if (separator != std::string::npos) {
        auto name = line.substr(0, separator);
        auto value = line.substr(separator + 1);
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.pop_back();
        }
        static_cast<std::map<std::string, std::string>*>(userData)
            ->insert_or_assign(std::move(name), std::move(value));
    }
    return length;
}

class CurlGlobal final {
public:
    CurlGlobal()
    {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            throw TransportError{"Failed to initialize libcurl."};
        }
    }

    ~CurlGlobal()
    {
        curl_global_cleanup();
    }
};

class CurlTransport final : public HttpTransport {
public:
    HttpResponse Post(const HttpRequest& request) override
    {
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl{
            curl_easy_init(), &curl_easy_cleanup};
        if (!curl) {
            throw TransportError{"Failed to create a libcurl request."};
        }

        curl_slist* headerList = nullptr;
        for (const auto& [name, value] : request.headers) {
            headerList = curl_slist_append(headerList, (name + ": " + value).c_str());
        }
        std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers{
            headerList, &curl_slist_free_all};

        HttpResponse response;
        char errorBuffer[CURL_ERROR_SIZE]{};

        curl_easy_setopt(curl.get(), CURLOPT_URL, request.url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, request.body.data());
        curl_easy_setopt(
            curl.get(), CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(request.body.size()));
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteBody);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);
        curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, WriteHeader);
        curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &response.headers);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 10'000L);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 120'000L);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer);

        const CURLcode result = curl_easy_perform(curl.get());
        if (result != CURLE_OK) {
            const std::string detail =
                errorBuffer[0] == '\0' ? curl_easy_strerror(result) : errorBuffer;
            throw TransportError{"Foundry request failed: " + detail};
        }

        long statusCode = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &statusCode);
        response.statusCode = static_cast<int>(statusCode);
        return response;
    }
};

CurlGlobal& GlobalCurl()
{
    static CurlGlobal instance;
    return instance;
}

} // namespace

std::shared_ptr<HttpTransport> MakeCurlTransport()
{
    (void)GlobalCurl();
    return std::make_shared<CurlTransport>();
}

} // namespace foundry_agent
