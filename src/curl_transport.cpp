#include "syslocker/nightflyer.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <cctype>
#include <mutex>

namespace syslocker::nightflyer
{
    namespace
    {
        std::once_flag initialized;

        std::size_t appendBody(char* bytes, std::size_t size, std::size_t count, void* context)
        {
            auto& body = *static_cast<std::string*>(context);
            const auto length = size * count;
            if (body.size() + length > 1048576) return 0;
            body.append(bytes, length);
            return length;
        }

        std::size_t appendHeader(char* bytes, std::size_t size, std::size_t count, void* context)
        {
            const auto length = size * count;
            std::string_view line(bytes, length);
            const auto colon = line.find(':');
            if (colon != std::string_view::npos)
            {
                std::string name(line.substr(0, colon));
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
                auto value = line.substr(colon + 1);
                while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
                while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.remove_suffix(1);
                (*static_cast<std::map<std::string, std::string>*>(context))[std::move(name)] = std::string(value);
            }
            return length;
        }

        class Curl final : public IHttpClient
        {
        public:
            explicit Curl(std::chrono::milliseconds timeout) : timeout_(timeout)
            {
                std::call_once(initialized, [] { if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) throw Error(Failure::local_failure, "libcurl global initialization failed."); });
            }

            HttpResponse postJson(std::string_view url, std::string_view body, std::string_view proof) override
            {
                HttpResponse response;
                CURL* curl = curl_easy_init();
                if (!curl) { response.error = "libcurl initialization failed."; return response; }
                curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                headers = curl_slist_append(headers, "Accept: application/json");
                headers = curl_slist_append(headers, "Cache-Control: no-store");
                const auto proofHeader = "Nightflyer-Proof: " + std::string(proof);
                headers = curl_slist_append(headers, proofHeader.c_str());
                const std::string ownedUrl(url);

                curl_easy_setopt(curl, CURLOPT_URL, ownedUrl.c_str());
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(body.size()));
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendBody);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, appendHeader);
                curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
                curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
                curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_.count()));
                curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(timeout_.count()));
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "systemlocker-nightflyer-cpp/0.2.0");

                const auto result = curl_easy_perform(curl);
                if (result != CURLE_OK) response.error = curl_easy_strerror(result);
                else curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                return response;
            }
        private:
            std::chrono::milliseconds timeout_;
        };
    }

    std::shared_ptr<IHttpClient> makeCurlHttpClient(std::chrono::milliseconds timeout) { return std::make_shared<Curl>(timeout); }
}
