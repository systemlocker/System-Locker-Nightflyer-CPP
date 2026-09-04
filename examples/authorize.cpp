#include <iostream>
#include <syslocker/nightflyer.hpp>

int main()
{
    syslocker::nightflyer::Config config;
    config.systemId = "abcdefghijklmnopqrst";
    config.trustedSigningKeys["0123456789ABCDEFGHJKMNPQRS"] = "replace-with-a-32-byte-base64url-ed25519-key";
    auto session = syslocker::nightflyer::AuthorizationSession::memory(std::move(config));
    auto result = session.easyAuthorize({"1.0.0", "", "your-hwid"}, 86400, "SL-REPLACE");
    std::cout << (result.canProceed ? result.snapshot.lease->compact : "authorization required") << '\n';
}
