# Third-party notices

Nightflyer uses the following third-party software:

- **libcurl**, under the curl license.
- **OpenSSL Crypto**, under the Apache License 2.0.
- **zlib**, under the zlib license. It is used by the static libcurl build.
- **JSON for Modern C++ (nlohmann/json)**, under the MIT License.

The prebuilt Windows SDK under `static/` contains compiled copies of these
components where applicable. Their complete license texts are included under
`static/licenses/`. No Nightflyer, curl, OpenSSL, or zlib DLLs are required at
runtime when linking the static package.
