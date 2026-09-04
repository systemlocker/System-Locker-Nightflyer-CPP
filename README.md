# System Locker Nightflyer — C++

Official C++20 client for System Locker Nightflyer offline authorization
leases. It signs each request with an installation P-256 key and accepts only
Ed25519-signed, locally pinned Nightflyer objects.

There are three supported ways to integrate the library into your application:

1. **Embed the source** — recommended. Add this repository as a CMake
   subdirectory or compile its sources as part of your own build.
2. **Use the prebuilt static SDK** — link the reviewed Windows x64 Release
   libraries in `static/` without installing Nightflyer's dependencies.
3. **Build and install the static library** — useful when you need another
   platform, architecture, compiler, or runtime-library choice.

Embedding keeps the implementation under your existing compiler settings and
makes it easy to debug in your application. A static library is a good fit
when your build or distribution process prefers a stable binary boundary.

## Requirements

CMake 3.20+, OpenSSL 3, libcurl, and nlohmann_json 3.11 are required. On
Windows, Nightflyer additionally uses the Windows Crypto APIs for persistent
state and installation keys.

## Embed the source (recommended)

Add Nightflyer to your source tree and let its CMake target participate in
your existing build. This preserves your compiler, runtime, and dependency
choices while providing the same `SystemLocker::Nightflyer` target used by an
installed package.

```cmake
set(SYSLOCKER_NIGHTFLYER_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/SystemLockerNightflyer)
target_link_libraries(my_app PRIVATE SystemLocker::Nightflyer)
```

The target compiles these implementation files and exposes `include/` as its
public include directory:

```text
src/nightflyer.cpp
src/curl_transport.cpp
src/authorization_session.cpp
src/dpapi_state_store.cpp
src/windows_cng_key_provider.cpp
```

It links libcurl, OpenSSL Crypto, and nlohmann_json. On Windows it also links
`Crypt32`, `Ncrypt`, `Bcrypt`, and `Ole32`. If you integrate without CMake,
compile the same source files as C++20, add `include/` to the include path, and
link those dependencies through your build system.

## Prebuilt Windows static SDK

The repository includes a self-contained Windows x64 SDK under `static/` and
as `systemlocker-nightflyer-sdk-0.1.0-win64-static.zip`. It contains the public
header, Nightflyer library, static libcurl/OpenSSL/zlib dependencies, and their
license texts. See [STATIC-LIBRARY.md](STATIC-LIBRARY.md) for Visual Studio
link settings and the complete package contract.

The archive is validated by a separate consumer project every time the
workspace packaging build refreshes it. No Nightflyer, curl, OpenSSL, or zlib
DLL is required at runtime.

## Build and install from source

The default CMake configuration also builds a static library from source.
Configure, build, and install it to a prefix your application can discover:

```powershell
cmake -S . -B build -DSYSLOCKER_NIGHTFLYER_STATIC=ON
cmake --build build --config Release
cmake --install build --config Release --prefix C:/local/SystemLockerNightflyer
```

Then configure your application with `CMAKE_PREFIX_PATH` pointing at that
prefix and link the installed CMake target:

```cmake
find_package(SystemLockerNightflyer CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE SystemLocker::Nightflyer)
```

The installed package still requires compatible libcurl, OpenSSL Crypto, and
nlohmann_json packages. Keep the compiler, architecture, runtime library, and
dependency linkage compatible with your application—especially when using a
static MSVC runtime.

## Quickstart

```cpp
#include <syslocker/nightflyer.hpp>
using namespace syslocker::nightflyer;

Config config;
config.systemId = "abcdefghijklmnopqrst";
config.trustedSigningKeys["YOUR_KID"] = "YOUR_BASE64URL_ED25519_KEY";
auto session = AuthorizationSession::memory(config);
auto result = session.easyAuthorize({"1.0.0", "", myHwid}, 86400, enteredKey);
if (result.canProceed)
    startProtectedWork(*result.snapshot.lease);
else if (result.shouldPromptForKey)
    showLicenseKeyPrompt();
```

For persistent Windows leases, use AuthorizationSession::persistent with a
Windows DPAPI state store. The protected record atomically stores the
installation key, lease, retained license key, time checkpoint, signing-key
transitions, and status/generation high-water marks. The retained key lets an
inactive installation request a fresh lease without asking the user to type it
again. Nightflyer erases it only after the server returns the definitive
`KEY_NOT_FOUND` result. On macOS or Linux, provide an IStateStore
implemented with that platform's OS key store; the library has no plaintext
fallback.

## EasyAuthorize and persistent startup

`easyAuthorize()` is the recommended startup API. With a newly entered key it
requests a fresh lease. Without one, it loads protected state, renews a due
lease or calls `synchronizeStatus()` for a strict status check, finishes any
exact pending end request, and requests a new lease with the retained key when
the old lease is inactive, ended, or expired.

The result exposes `canProceed`, `isOffline`, `credentialPresent`,
`shouldPromptForKey`, `retryAfter`, and a typed `recommendedAction`. A
`network_unavailable` snapshot with `canProceed == true` means the online check
did not finish but the existing signed offline lease remains usable.

If a developer **unbinds** a lease, the old lease is reported as revoked and
surfaces locally as `inactive`; this is not a permanent ban on the installation
or key. `easyAuthorize()` immediately tries to obtain a new lease with the
retained key. `KEY_NOT_FOUND` clears the retained key and requests another;
other denials retain it while still allowing the user to enter a replacement.

```cpp
auto startup = session.easyAuthorize(binding, 86400);
if (!startup.canProceed)
    handle(startup.recommendedAction, startup.snapshot.diagnostic, startup.retryAfter);
```

Alternatively, the lower-level `load()`, `renewIfDue()`, `synchronizeStatus()`,
and `authorizeWithStoredKey()` operations are available for any application that
needs more control over the process.

## Offline time and travel

A valid cached lease can be used without connectivity, including while
traveling. During the same OS boot, lease time advances from verified server
UTC using an elapsed-time clock that includes sleep. Time-zone changes and
wall-clock corrections do not shorten or extend the signed lease lifetime.

After a reboot, the default policy permits offline use when UTC is consistent
with the protected time checkpoint and the lease has not expired. A clock
rollback that cannot be verified requires an online check; set
`requireOnlineAfterReboot` to require one after every reboot instead. A
successful signed online response repairs a bad local time checkpoint but
does not extend an existing lease's signed expiry. If the OS cannot provide a
boot identifier, reopening the application uses the same conservative UTC
checks as a reboot.

## Security addendum

Pinned server keys are a developer-provided trust root. The session accepts a
new key only through a signed transition from a previously trusted key, persists
the key/status/generation high-water marks, and rejects revoked keys, devices,
grants, rollback, and stale leases. It uses the signed server-time anchor plus a
suspend-aware monotonic clock within a boot. Nightflyer cannot immediately
revoke a device that is offline, and even though it increases resilience to
cracking, it is not a substitute for proper obfuscation and other protections.
