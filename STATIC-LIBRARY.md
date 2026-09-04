# Prebuilt static library

System Locker publishes a self-contained Windows x64 SDK for developers who
want to link Nightflyer without building its third-party dependencies. The
package is built with Microsoft Visual C++, the static MSVC runtime, and static
copies of libcurl, OpenSSL Crypto, and zlib. It does not require Nightflyer,
curl, OpenSSL, or zlib DLLs at runtime.

The current release archive is named
`systemlocker-nightflyer-sdk-0.1.0-win64-static.zip` and is also available as
the `static/` directory in this repository.

## Visual Studio integration

1. Add the SDK's `include` directory (`static/include` in this repository) to
   **Additional Include Directories**.
2. Add the SDK's `lib` directory (`static/lib` in this repository) to
   **Additional Library Directories**.
3. Use C++20 and target x64.
4. Select the static runtime: `/MT` for Release.
5. Add these libraries to **Additional Dependencies**:

```text
systemlocker_nightflyer.lib
libcurl.lib
libcrypto.lib
zs.lib
crypt32.lib
ncrypt.lib
bcrypt.lib
ole32.lib
advapi32.lib
secur32.lib
ws2_32.lib
iphlpapi.lib
user32.lib
```

The published binary is a Release build. Applications that require a Debug
runtime build should compile the published source with the matching toolchain.

## Package contents

```text
include/       Nightflyer's public C++ header
lib/           Nightflyer and bundled dependency libraries
licenses/      curl, OpenSSL, zlib, and nlohmann/json license texts
README.md
SECURITY.md
THIRD_PARTY_NOTICES.md
```

The package is supported for Windows x64 applications built with a compatible
Microsoft Visual C++ toolset. Source and CMake package installation remain
available for other platforms and toolchains.
