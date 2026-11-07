#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

// Minimal HTTPS client (WinHTTP) for the catalog and module downloads. Follows redirects.
namespace http {

bool Get(const std::string& url, std::string& body, std::string& error);

using Progress = std::function<void(uint64_t done, uint64_t total)>;
bool Download(const std::string& url, const std::filesystem::path& file, const Progress& progress,
              std::string& error);

// Lowercase hex SHA-256 of a file, or empty on error.
std::string Sha256File(const std::filesystem::path& file);

}  // namespace http
