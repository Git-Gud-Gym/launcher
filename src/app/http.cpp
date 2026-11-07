#include "http.h"

#include <windows.h>
#include <bcrypt.h>
#include <winhttp.h>

#include <cstdio>
#include <fstream>
#include <vector>

#include "version.h"

namespace http {

namespace {

std::wstring Widen(const std::string& s) {
    std::wstring w(s.size(), L'\0');
    w.resize(MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(),
                                 static_cast<int>(w.size())));
    return w;
}

std::string Narrow(const std::wstring& w) {
    std::string s(w.size() * 3, '\0');
    s.resize(WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), s.data(),
                                 static_cast<int>(s.size()), nullptr, nullptr));
    return s;
}

struct Handle {
    HINTERNET h = nullptr;
    ~Handle() {
        if (h) WinHttpCloseHandle(h);
    }
};

// Runs a GET and feeds the body to sink in chunks. sink returns false to abort.
bool Fetch(const std::string& url, const std::function<bool(const char*, size_t, uint64_t total)>& sink,
           std::string& error) {
    const std::wstring wurl = Widen(url);
    URL_COMPONENTS parts{sizeof(parts)};
    wchar_t host[256]{}, path[2048]{};
    parts.lpszHostName = host;
    parts.dwHostNameLength = static_cast<DWORD>(std::size(host));
    parts.lpszUrlPath = path;
    parts.dwUrlPathLength = static_cast<DWORD>(std::size(path));
    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &parts) || parts.nScheme != INTERNET_SCHEME_HTTPS) {
        error = "Bad URL (only https is allowed): " + url;
        return false;
    }

    Handle session{WinHttpOpen(L"GitGudGym/" GGG_V_STRING, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                               WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)};
    if (!session.h) {
        error = "Couldn't start WinHTTP";
        return false;
    }
    WinHttpSetTimeouts(session.h, 10000, 10000, 15000, 30000);
    Handle connect{WinHttpConnect(session.h, host, parts.nPort, 0)};
    Handle request{connect.h ? WinHttpOpenRequest(connect.h, L"GET", path, nullptr, WINHTTP_NO_REFERER,
                                                  WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE)
                             : nullptr};
    if (!request.h || !WinHttpSendRequest(request.h, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request.h, nullptr)) {
        error = "Couldn't reach " + Narrow(host) + " (error " +
                std::to_string(GetLastError()) + ")";
        return false;
    }

    DWORD status = 0, size = sizeof(status);
    WinHttpQueryHeaders(request.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size,
                        nullptr);
    if (status != 200) {
        error = "HTTP " + std::to_string(status) + " for " + url;
        return false;
    }
    uint64_t total = 0;
    wchar_t lengthText[32]{};
    DWORD lengthSize = sizeof(lengthText);
    if (WinHttpQueryHeaders(request.h, WINHTTP_QUERY_CONTENT_LENGTH, nullptr, lengthText, &lengthSize, nullptr)) {
        total = _wcstoui64(lengthText, nullptr, 10);
    }

    std::vector<char> buf(64 * 1024);
    for (;;) {
        DWORD read = 0;
        if (!WinHttpReadData(request.h, buf.data(), static_cast<DWORD>(buf.size()), &read)) {
            error = "Download interrupted (error " + std::to_string(GetLastError()) + ")";
            return false;
        }
        if (read == 0) return true;
        if (!sink(buf.data(), read, total)) {
            error = "Couldn't write the download to disk";
            return false;
        }
    }
}

}  // namespace

bool Get(const std::string& url, std::string& body, std::string& error) {
    body.clear();
    return Fetch(
        url,
        [&](const char* data, size_t n, uint64_t) {
            body.append(data, n);
            return body.size() < 16 * 1024 * 1024;  // catalogs are tiny; refuse anything absurd
        },
        error);
}

bool Download(const std::string& url, const std::filesystem::path& file, const Progress& progress,
              std::string& error) {
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "Couldn't create " + file.string();
        return false;
    }
    uint64_t done = 0;
    const bool ok = Fetch(
        url,
        [&](const char* data, size_t n, uint64_t total) {
            out.write(data, static_cast<std::streamsize>(n));
            done += n;
            if (progress) progress(done, total);
            return static_cast<bool>(out);
        },
        error);
    out.close();
    return ok && out;
}

std::string Sha256File(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return {};
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return {};
    std::string hex;
    if (BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0) == 0) {
        std::vector<char> buf(64 * 1024);
        while (in) {
            in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
            if (in.gcount() > 0) {
                BCryptHashData(hash, reinterpret_cast<PUCHAR>(buf.data()), static_cast<ULONG>(in.gcount()), 0);
            }
        }
        UCHAR digest[32];
        if (BCryptFinishHash(hash, digest, sizeof(digest), 0) == 0) {
            char part[3];
            for (UCHAR b : digest) {
                std::snprintf(part, sizeof(part), "%02x", b);
                hex += part;
            }
        }
        BCryptDestroyHash(hash);
    }
    BCryptCloseAlgorithmProvider(alg, 0);
    return hex;
}

}  // namespace http
